/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/
#include "tool_setup.h"
#include "tool_sync.h"
#include "tool_cfgable.h"
#include "tool_sdecls.h"
#include "tool_operate.h"  /* for struct GlobalConfig */

#include "memdebug.h" /* keep this as LAST include */

/* External reference to global config */
extern struct GlobalConfig *global;

/*
 * Check if we need to split execution into batches.
 * Returns TRUE if there are multiple URLs with different sync states.
 */
bool sync_should_split_batches(struct OperationConfig *config)
{
  struct getout *url;
  bool has_sync = FALSE;
  bool has_nonsync = FALSE;

  if(!config || !config->url_list)
    return FALSE;

  /* Check if we have both sync and non-sync URLs */
  for(url = config->url_list; url; url = url->next) {
    if(!url->url)
      continue;

    if(url->sync)
      has_sync = TRUE;
    else
      has_nonsync = TRUE;

    /* If we have both types, we need batching */
    if(has_sync && has_nonsync)
      return TRUE;
  }

  return FALSE;
}

/*
 * Execute URL list in batches based on sync state changes.
 * Each batch contains consecutive URLs with the same sync state.
 */
CURLcode sync_execute_batches(struct OperationConfig *config,
                              CURLSH *share,
                              CURLcode (*execute_func)(CURLSH *share))
{
  CURLcode result = CURLE_OK;
  struct getout *batch_start = NULL;
  struct getout *url;
  struct getout *saved_url_list;
  struct OperationConfig *saved_current;
  struct OperationConfig *saved_next;
  bool current_batch_sync;
  /* Save original config state to restore between batches */
  bool saved_sync;
  bool saved_remote_time;
  unsigned char saved_fail;
  unsigned char saved_timecond;

  if(!config || !config->url_list || !execute_func)
    return CURLE_BAD_FUNCTION_ARGUMENT;

  saved_url_list = config->url_list;
  saved_current = global->current;
  saved_next = config->next;
  saved_sync = config->sync;
  saved_remote_time = config->remote_time;
  saved_fail = config->fail;
  saved_timecond = config->timecond;

  /* Process URLs in batches grouped by sync state */
  for(url = config->url_list; url; ) {
    struct getout *batch_end = url;
    struct getout *next_batch;

    /* Skip URLs without actual URL set */
    while(batch_end && !batch_end->url)
      batch_end = batch_end->next;

    if(!batch_end)
      break;

    batch_start = batch_end;
    current_batch_sync = batch_start->sync;

    /* Find end of current batch (consecutive URLs with same sync state) */
    while(batch_end->next) {
      struct getout *check = batch_end->next;

      /* Skip URLs without URL set */
      if(!check->url) {
        batch_end = check;
        continue;
      }

      /* If sync state changes, start new batch */
      if(check->sync != current_batch_sync)
        break;

      batch_end = check;
    }

    /* Save pointer to next batch */
    next_batch = batch_end->next;

    /* Temporarily terminate URL list at batch boundary */
    batch_end->next = NULL;

    /* Set config to point to this batch */
    config->url_list = batch_start;

    /* Temporarily terminate config chain so only this config is processed */
    config->next = NULL;
    global->current = config;

    /* Set sync state for this batch */
    config->sync = current_batch_sync;
    if(current_batch_sync) {
      config->remote_time = TRUE;
      config->fail = FAIL_WO_BODY;
      config->timecond = CURL_TIMECOND_IFMODSINCE;
    }
    else {
      config->remote_time = FALSE;
      config->fail = FAIL_NONE;
      config->timecond = CURL_TIMECOND_NONE;
    }

    /* Execute this batch */
    result = execute_func(share);

    /* Restore URL list linkage for next batch */
    batch_end->next = next_batch;

    /* Restore config state for next batch */
    config->sync = saved_sync;
    config->remote_time = saved_remote_time;
    config->fail = saved_fail;
    config->timecond = saved_timecond;

    /* If error occurred, stop processing */
    if(result)
      break;

    /* Move to next batch */
    url = next_batch;
  }

  /* Restore original URL list and config chain */
  config->url_list = saved_url_list;
  config->next = saved_next;
  global->current = saved_current;

  return result;
}
