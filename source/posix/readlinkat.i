/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2026 Carsten Janssen
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
#include <wchar.h>
#include <errno.h>
#include <stdlib.h>
#include "w32-symlink-posix.h"
#include "common.h"
#include "helper.h"


ssize_t _w(readlinkat)(int dirfd, const xchar_t *path, xchar_t *buf, size_t numcs)
{
    xchar_t *buf_path, *ptr;
    int errsav;

    if (!path || !*path || !buf || numcs == 0) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    if (numcs > SSIZE_MAX) {
        numcs = SSIZE_MAX;
    }

    /* if special value AT_FDCWD is used the behavior is exactly like readlink() */
    if (dirfd == AT_FDCWD || _w(private_is_absolute_path)(path)) {
        ptr = _w(readlink_s)(path, buf, numcs);

        return ptr ? (ssize_t)xstrlen(buf) : -1;
    }

    /* create full path; fails if dirfd doesn't belong to a directory */
    buf_path = _w(private_create_path_from_dirfd)(dirfd, path);

    if (!buf_path) {
        /* errno is set */
        return -1;
    }

    /* call readlink_s() */
    ptr = _w(readlink_s)(buf_path, buf, numcs);

    errsav = errno;
    free(buf_path);
    errno = errsav;

    return ptr ? (ssize_t)xstrlen(buf) : -1;
}


xchar_t *_w(readlinkat_s)(int dirfd, const xchar_t *path, xchar_t *buf, size_t numcs)
{
    xchar_t *buf_path, *ptr;
    int errsav;

    if (!path || !*path || (buf && numcs == 0)) {
        errno = EINVAL; /* Invalid argument */
        return NULL;
    }

    /* if special value AT_FDCWD is used the behavior is exactly like readlink_s() */
    if (dirfd == AT_FDCWD || _w(private_is_absolute_path)(path)) {
        return _w(readlink_s)(path, buf, numcs);
    }

    /* create full path; fails if dirfd doesn't belong to a directory */
    buf_path = _w(private_create_path_from_dirfd)(dirfd, path);

    if (!buf_path) {
        /* errno is set */
        return NULL;
    }

    /* call readlink_s() */
    ptr = _w(readlink_s)(buf_path, buf, numcs);

    errsav = errno;
    free(buf_path);
    errno = errsav;

    return ptr;
}

