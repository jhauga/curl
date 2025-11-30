---
c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: sync
Help: Download with remote name if newer (shorthand)
Category: output
Added: 8.18.0
Multi: boolean
See-also:
  - remote-name
  - remote-time
  - time-cond
  - location
Example:
  - --sync $URL/file.txt
  - --sync $URL/file1.txt --sync $URL/file2.txt
  - --sync -L $URL/file.txt
---

# `--sync`

This option applies **per-URL**. Each URL that should use sync behavior must
have --sync specified before it.

Additionally, --sync causes HTTP error responses (4xx/5xx) to be treated as
errors, preventing incomplete files from being saved. Redirects (3xx) are
allowed only when combined with --location/-L. The 304 Not Modified response
is treated as success, indicating the local file is already current.

If the local file does not exist, it is downloaded (if HTTP 200 is returned).
If it exists and the remote file is newer (based on Last-Modified header), the
local file is updated (HTTP 200). If the local file is already up to date, the
server returns 304 Not Modified and no download occurs.

The file is saved in the current working directory unless --output-dir is
used. Disable it again with --no-sync.
