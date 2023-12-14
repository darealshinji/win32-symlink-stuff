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
#ifndef _GETCANONICALPATH_H_
#define _GETCANONICALPATH_H_

#include <windows.h>

#ifdef _UNICODE
#define getCanonicalPath getCanonicalPathW
#else
#define getCanonicalPath getCanonicalPathA
#endif

/**
 * getCanonicalPath() returns the canonicalized absolute path form
 * of lpFileName, with all symbolic links and '.' and '..' elements resolved.
 * This function is similar to POSIX's `realpath(3)` or GNU's
 * `canonicalize_file_name(3)`.
 *
 * Consecutive path separators are replaced with a single '\'.
 * The resulting path will begin with "\\?\" followed by the drive letter.
 * 
 * The result must be deallocated with free().
 */
char    *getCanonicalPathA(const char *lpFileName);
wchar_t *getCanonicalPathW(const wchar_t *lpFileName);

#endif /* _GETCANONICALPATH_H_ */
