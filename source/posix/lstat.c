/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2025-2026 Carsten Janssen
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
#include <errno.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "w32-symlink-posix.h"
#include "convert.h"


/**
 * create the different implementations of lstat functions as wrappers
 * around _lstat64 and _lwstat64
 *
 * https://learn.microsoft.com/cpp/c-runtime-library/reference/stat-functions
 */


#define MAKE_FUNC(FUNC, LSTAT64, XCHAR, STATBUF, TIME, SIZE) \
    int FUNC(const XCHAR *path, struct STATBUF *sb) \
    { \
        struct _stat64 st; \
        int rv; \
        \
        /* need to check at least 'sb' here, 'path' is */ \
        /* also checked by the stat functions */ \
        if (!path || !*path || !sb) { \
            errno = EINVAL; \
            return -1; \
        }  \
        \
        if ((rv = LSTAT64(path, &st)) == 0) { \
            sb->st_dev   = st.st_dev; \
            sb->st_ino   = st.st_ino; \
            sb->st_mode  = st.st_mode; \
            sb->st_nlink = st.st_nlink; \
            sb->st_uid   = st.st_uid; \
            sb->st_gid   = st.st_gid; \
            sb->st_rdev  = st.st_rdev; \
            sb->st_size  = (SIZE)(st.st_size); \
            sb->st_atime = (TIME)(st.st_atime); \
            sb->st_mtime = (TIME)(st.st_mtime); \
            sb->st_ctime = (TIME)(st.st_ctime); \
        } \
        return rv; \
    }

#define IMPLEMENT_LSTAT(LSTAT, LWSTAT, STATBUF, TIME, SIZE) \
    MAKE_FUNC(LSTAT,  _lstat64,  char,    STATBUF, TIME, SIZE) \
    MAKE_FUNC(LWSTAT, _lwstat64, wchar_t, STATBUF, TIME, SIZE)

IMPLEMENT_LSTAT ( lstat,       lwstat,       stat,        time_t,    _off_t)
IMPLEMENT_LSTAT (_lstat,      _lwstat,      _stat,        time_t,    _off_t)
IMPLEMENT_LSTAT (_lstat32,    _lwstat32,    _stat32,    __time32_t,  _off_t)
IMPLEMENT_LSTAT (_lstati64,   _lwstati64,   _stati64,     time_t,   __int64)
IMPLEMENT_LSTAT (_lstat32i64, _lwstat32i64, _stat32i64, __time32_t, __int64)
IMPLEMENT_LSTAT (_lstat64i32, _lwstat64i32, _stat64i32, __time64_t,  _off_t)

