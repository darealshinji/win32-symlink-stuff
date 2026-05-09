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
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <wchar.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "w32-symlink.h"
#include "common.h"
#include "helper.h"


#if defined(UTF8_EVERYWHERE) || defined(WIDE_CHAR_API)

int _w(symlink)(const xchar_t *target, const xchar_t *linkpath)
{
    char mode = 0;
    DWORD dwAttr;

    if (!target || !*target || !linkpath || !*linkpath) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    dwAttr = AW(GetFileAttributes)(target);

    /* set mode if target exists and is a directory */
    if (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        mode = 'D';
    }

    if (AW(createLink)(linkpath, target, mode) == FALSE) {
        errno = private_map_winerr_to_errno(GetLastError());
        return -1;
    }

    return 0;
}

#endif


int _w(link)(const xchar_t *oldpath, const xchar_t *newpath)
{
    if (!oldpath || !*oldpath || !newpath || !*newpath) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    if (AW(createLink)(oldpath, newpath, 'H') == FALSE) {
        errno = private_map_winerr_to_errno(GetLastError());
        return -1;
    }

    return 0;
}


ssize_t _w(readlink)(const xchar_t *path, xchar_t *buf, size_t numcs)
{
    xchar_t *ptr;

    if (!path || !*path || !buf || numcs == 0) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    if (numcs > SSIZE_MAX) {
        numcs = SSIZE_MAX;
    }

    ptr = _w(readlink_s)(path, buf, numcs);

    if (!ptr) {
        return -1;
    }

    return (ssize_t)xstrlen(buf);
}


xchar_t *_w(readlink_s)(const xchar_t *path, xchar_t *buf, size_t numcs)
{
    xchar_t *ptr;

    if (!path || !*path || (buf && numcs == 0)) {
        errno = EINVAL; /* Invalid argument */
        return NULL;
    }

    ptr = AW(getLinkTarget)(path, NULL);

    return _w(private_return_path)(ptr, buf, numcs);
}


xchar_t *_w(realpath_s)(const xchar_t *path, xchar_t *buf, size_t numcs)
{
    xchar_t *ptr;

    if (!path || !*path || (buf && numcs == 0)) {
        errno = EINVAL; /* Invalid argument */
        return NULL;
    }

    ptr = AW(getCanonicalPath)(path);

    return _w(private_return_path)(ptr, buf, numcs);
}


xchar_t *_w(realpath)(const xchar_t *path, xchar_t *resolved_path)
{
    xchar_t buf[PATH_MAX];
    xchar_t *ptr;
    size_t len;

    ptr = _w(realpath_s)(path, buf, PATH_MAX);

    if (!ptr) {
        /* error (including truncation from exceeding PATH_MAX) */
        return NULL;
    }

    if (!resolved_path) {
        /* return allocated copy */
        return _xstrdup(buf);
    }

    len = xstrlen(buf);
    xmemcpy_s(resolved_path, PATH_MAX, buf, len);
    resolved_path[len] = 0;

    return resolved_path;
}


xchar_t *_w(canonicalize_file_name)(const xchar_t *path)
{
    return _w(realpath_s)(path, NULL, 0);
}

