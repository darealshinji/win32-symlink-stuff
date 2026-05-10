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
#include "w32-symlink.h"
#include "common.h"
#include "helper.h"


int _w(linkat)(int olddirfd, const xchar_t *oldpath,
               int newdirfd, const xchar_t *newpath, int flags)
{
    const xchar_t *ptr_oldpath = NULL;
    const xchar_t *ptr_newpath = NULL;
    xchar_t *buf_oldpath = NULL;
    xchar_t *buf_newpath = NULL;
    xchar_t *resolved = NULL;
    int errsav;
    int ret = -1;

    /* flags must be 0! */
    if (!oldpath || !*oldpath || !newpath || !*newpath) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    /* if special value AT_FDCWD is used or the path is absolute,
     * the behavior is exactly like link() */
    if (olddirfd == AT_FDCWD || _w(private_is_absolute_path)(oldpath)) {
        ptr_oldpath = oldpath;
    } else {
        /* create full path; fails if file descriptor doesn't belong to a directory */
        ptr_oldpath = buf_oldpath = _w(private_create_path_from_dirfd)(olddirfd, oldpath);

        if (!buf_oldpath) {
            /* errno is set */
            goto jump_cleanup;
        }
    }

    if (newdirfd == AT_FDCWD || _w(private_is_absolute_path)(newpath)) {
        ptr_newpath = newpath;
    } else {
        ptr_newpath = buf_newpath = _w(private_create_path_from_dirfd)(newdirfd, newpath);

        if (!buf_newpath) {
            /* errno is set */
            goto jump_cleanup;
        }
    }

    /* handle case where oldpath is a symbolic link */
    if (AW(isSymlink)(ptr_oldpath, NULL) == TRUE) {
        if (!(flags & AT_SYMLINK_FOLLOW)) {
            /* don't follow symbolic links */
            errno = EPERM;
            goto jump_cleanup;
        }

        if ((resolved = AW(getCanonicalPath)(ptr_oldpath)) == NULL) {
            errno = private_map_winerr_to_errno(GetLastError());
            goto jump_cleanup;
        }

        ptr_oldpath = resolved;
    }

    /* call link() */
    ret = _w(link)(ptr_oldpath, ptr_newpath);

jump_cleanup:
    errsav = errno;
    free(buf_oldpath);
    free(buf_newpath);
    free(resolved);
    errno = errsav;

    return ret;
}

