/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2023-2026 Carsten Janssen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE
 */
#undef _UNICODE
#undef UNICODE
#include <windows.h>
#include <errno.h>
#include <wchar.h>
#include <limits.h>
#include <stdlib.h>
#include "w32-symlink.h"
#include "w32-symlink-posix.h"
#include "common.h"
#include "helper.h"

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH /* 260 characters */
#endif



xchar_t *_w(realpath)(const xchar_t *path, xchar_t *resolved_path)
{
    xchar_t *ptr;
    size_t len;

    if (!path || !*path) {
        errno = EINVAL; /* Invalid argument */
        return NULL;
    }

    ptr = AW(getCanonicalPath)(path); /* returns allocated string */

    if (!ptr) {
        errno = private_map_winerr_to_errno(GetLastError());
        return NULL;
    }

    len = xstrlen(ptr);

    if (len >= PATH_MAX) {
        /* pathname exceeded PATH_MAX (including terminating NUL character) */
        free(ptr);
        errno = ENAMETOOLONG;
        return NULL;
    }

    if (!resolved_path) {
        return ptr; /* return allocated string */
    }

    xmemcpy_s(resolved_path, PATH_MAX, ptr, len + 1);
    free(ptr);

    return resolved_path;
}


xchar_t *_w(canonicalize_file_name)(const xchar_t *path)
{
    xchar_t *ptr;

    if (!path || !*path) {
        errno = EINVAL; /* Invalid argument */
        return NULL;
    }

    ptr = AW(getCanonicalPath)(path); /* returns allocated string */

    if (!ptr) {
        errno = private_map_winerr_to_errno(GetLastError());
        return NULL;
    }

    return ptr;
}
