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
#include <windows.h>
#include <tchar.h>

#include "createLink.h"

/* https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createsymboliclinkw */
#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
#define SYMBOLIC_LINK_FLAG_DIRECTORY  0x1
#endif
#ifndef SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
#define SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE  0x2
#endif

#ifndef CreateSymbolicLink
extern BOOLEAN CreateSymbolicLinkA(LPCSTR lpLinkName, LPCSTR lpTargetName, DWORD dwFlags);
extern BOOLEAN CreateSymbolicLinkW(LPCWSTR lpLinkName, LPCWSTR lpTargetName, DWORD dwFlags);
#ifdef _UNICODE
#define CreateSymbolicLink CreateSymbolicLinkW
#else
#define CreateSymbolicLink CreateSymbolicLinkA
#endif
#endif


BOOL createLink(const _TCHAR *lpLinkName, const _TCHAR *lpTargetName, char mode)
{
    DWORD dwFlags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

    if (mode == 'h' || mode == 'H') {
        /* hard link */
        return CreateHardLink(lpLinkName, lpTargetName, NULL);
    } else if (mode == 'd' || mode == 'D') {
        /* symbolic link to directory */
        dwFlags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
    }

    /* create symbolic link */
    if (CreateSymbolicLink(lpLinkName, lpTargetName, dwFlags)) {
        return TRUE;
    }

    /* failure: remove this flag and try again */
    dwFlags &= ~SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;

    return CreateSymbolicLink(lpLinkName, lpTargetName, dwFlags);
}

