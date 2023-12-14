/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2023 djcj@gmx.de
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
#ifndef _ISSYMLINK_H_
#define _ISSYMLINK_H_

#include <windows.h>

#ifdef _UNICODE
#define isSymlink     isSymlinkW
#else
#define isSymlink     isSymlinkA
#endif

/**
 * isSymlink() will return TRUE if lpFileName is any type of symbolic link.
 *
 * If lpFileName exists and is NOT a symbolic link, it will return FALSE and
 * set the last error to ERROR_SUCCESS.
 *
 * If an error occurrs this function returns FALSE and the error code can be
 * retrieved with GetLastError().
 */
BOOL isSymlinkA(const char *lpFileName);
BOOL isSymlinkW(const wchar_t *lpFileName);

#endif /* _ISSYMLINK_H_ */
