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
#ifndef NO_OLDNAMES

/**
 * Build implementations of deprecated function names without preceeding
 * underscore but without causing compiler warnings.
 */

#include <windows.h>
#include <sys/types.h>

/* typedef for ssize_t */
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
#undef ssize_t
#ifdef _WIN64
typedef __int64 ssize_t;
#else
typedef int ssize_t;
#endif /* _WIN64 */
#endif /* _SSIZE_T_DEFINED */


/* prototypes */
ssize_t _readlink(const char *path, char *buf, size_t bufsize);
ssize_t _readlinkat(int dirfd, const char *path, char *buf, size_t bufsize);
char *_realpath(const char *path, char *resolved_path);


/* deprecated functions */
ssize_t readlink(const char *path, char *buf, size_t bufsize) {
    return _readlink(path, buf, bufsize);
}

ssize_t readlinkat(int dirfd, const char *path, char *buf, size_t bufsize) {
    return _readlinkat(dirfd, path, buf, bufsize);
}

char *realpath(const char *path, char *resolved_path) {
    return _realpath(path, resolved_path);
}

#endif /* !NO_OLDNAMES */
