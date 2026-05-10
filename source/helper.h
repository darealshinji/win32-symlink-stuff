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
#ifndef HELPER_H_INCLUDED
#define HELPER_H_INCLUDED

#ifdef WIDE_CHAR_API

#include <wchar.h>
typedef wchar_t  xchar_t;
#define _T(x)       L##x  /* string literals */
#define _w(x)       _w##x  /* prefix for C API functions */
#define AW(x)       x##W  /* suffix for Windows API functions */

/* basically TCHAR API */
#define xstrlen     wcslen
#define xstrnlen_s  wcsnlen_s
#define xstrncmp    wcsncmp
#define xstrncpy_s  wcsncpy_s
#define xisalpha    iswalpha
#define _xstrdup    _wcsdup
#define _xlstat64   _lwstat64
#define _xstat64    _wstat64
#define xmemcpy_s   wmemcpy_s

#else

typedef char xchar_t;
#define _T(x)       x
#define _w(x)       _##x
#define AW(x)       x##A

#define xstrlen     strlen
#define xstrnlen_s  strnlen_s
#define xstrncmp    strncmp
#define xstrncpy_s  strncpy_s
#define xisalpha    isalpha
#define _xstrdup    _strdup
#define _xlstat64   _lstat64
#define _xstat64    _stat64
#define xmemcpy_s   memcpy_s

#endif


#endif /* HELPER_H_INCLUDED */
