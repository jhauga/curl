#ifndef HEADER_CURL_TOOL_SYNC_H
#define HEADER_CURL_TOOL_SYNC_H
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

struct OperationConfig;

/*
 * Check if the URL list should be split into sync batches.
 * Returns TRUE if --sync is used with multiple URLs that have different
 * sync states, requiring batch processing.
 */
bool sync_should_split_batches(struct OperationConfig *config);

/*
 * Execute URL list in batches based on sync state changes.
 * Groups consecutive URLs with the same sync state and executes each
 * group as if it were a separate curl command invocation.
 *
 * This ensures --sync and --no-sync URLs are processed independently,
 * preventing state bleeding between different sync modes.
 */
CURLcode sync_execute_batches(struct OperationConfig *config,
                              CURLSH *share,
                              CURLcode (*execute_func)(CURLSH *share));

#endif /* HEADER_CURL_TOOL_SYNC_H */
