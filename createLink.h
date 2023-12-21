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
#ifndef _CREATELINK_H_
#define _CREATELINK_H_

#include <windows.h>

#ifdef _UNICODE
#define createLink createLinkW
#else
#define createLink createLinkA
#endif

/**
 * Creates a link lpLinkName to the target lpTargetName.
 * The type of link to create depends on the mode.
 * This function is similar to POSIX's `symlink(2)` and `link(2)`.
 *
 * If mode is 'h' or 'H' a hard link (new filename) will be created.
 * In this case lpLinkName will be the new filename and lpTargetName
 * is the old filename.
 *
 * If mode is 'd' or 'D' a symbolic link to a directory will be created.
 *
 * Any other value for mode will create a regular symbolic link.
 */
BOOL createLinkA(const char *lpLinkName, const char *lpTargetName, char mode);
BOOL createLinkW(const wchar_t *lpLinkName, const wchar_t *lpTargetName, char mode);

#endif /* _CREATELINK_H_ */
