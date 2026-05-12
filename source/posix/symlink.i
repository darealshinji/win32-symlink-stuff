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
#include <wchar.h>
#include <errno.h>
#include <stdlib.h>
#include "w32-symlink.h"
#include "w32-symlink-posix.h"
#include "common.h"
#include "convert.h"
#include "helper.h"


static BOOL target_is_directory(const xchar_t *target)
{
    DWORD dwAttr;
    wchar_t *wcs_target;

#if defined(UTF8_EVERYWHERE) || defined(WIDE_CHAR_API)

    dwAttr = AW(GetFileAttributes)(target);
    (void)wcs_target;

#else

    if ((wcs_target = convert_str_to_wcs(target)) == NULL) {
        return FALSE;
    }

    dwAttr = GetFileAttributesW(wcs_target);
    free(wcs_target);

#endif

    return (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}


int _w(symlink)(const xchar_t *target, const xchar_t *linkpath)
{
    char mode = 0;

    if (!target || !*target || !linkpath || !*linkpath) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    /* automatic detection if target is a directory */
    if (target_is_directory(target)) {
        mode = 'D';
    }

    if (AW(createLink)(linkpath, target, mode) != TRUE) {
        errno = private_map_winerr_to_errno(GetLastError());
        return -1;
    }

    return 0;
}


int _w(symlinkat)(const xchar_t *target, int newdirfd, const xchar_t *linkpath)
{
    xchar_t *buf_linkpath = NULL;
    int ret, errsav;

    if (!target || !*target || !linkpath || !*linkpath) {
        errno = EINVAL; /* Invalid argument */
        return -1;
    }

    /* if special value _AT_FDCWD is used or the link path is absolute,
     * the behavior is exactly like symlink() */
    if (newdirfd == _AT_FDCWD || _w(private_is_absolute_path)(linkpath)) {
        return _w(symlink)(target, linkpath);
    }

    /* create full path; fails if newdirfd doesn't belong to a directory */
    buf_linkpath = _w(private_create_path_from_dirfd)(newdirfd, linkpath);

    if (!buf_linkpath) {
        /* errno is set */
        return -1;
    }

    /* call symlink() */
    ret = _w(symlink)(target, buf_linkpath);

    errsav = errno;
    free(buf_linkpath);
    errno = errsav;

    return ret;
}

