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
#include <stdlib.h>
#include <stdio.h>

#include "getCanonicalPath.h"
#include "getLinkTarget.h"

#ifndef GetFinalPathNameByHandle
extern DWORD GetFinalPathNameByHandleA(HANDLE hFile, LPSTR lpszFilePath, DWORD cchFilePath, DWORD dwFlags);
extern DWORD GetFinalPathNameByHandleW(HANDLE hFile, LPWSTR lpszFilePath, DWORD cchFilePath, DWORD dwFlags);
#ifdef _UNICODE
#define GetFinalPathNameByHandle GetFinalPathNameByHandleW
#else
#define GetFinalPathNameByHandle GetFinalPathNameByHandleA
#endif
#endif


/**
 * Result must be deallocated with free().
 */
static _TCHAR *__getCanonicalPath(const _TCHAR *lpFileName)
{
    _TCHAR *buf = NULL;
    HANDLE hPath;
    DWORD dwLen;

    const DWORD dwFlags =
        FILE_NAME_NORMALIZED | /* Normalize the path. -> This is what we want! */
        VOLUME_NAME_DOS;       /* Return path with drive letter (uses "\\?\" syntax). */

    /* open lpFileName for reading */
    hPath = CreateFile(lpFileName,
                       0,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_FLAG_BACKUP_SEMANTICS,
                       NULL);

    if (hPath == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    /* figure out length */
    dwLen = GetFinalPathNameByHandle(hPath, NULL, 0, dwFlags);

    if (dwLen > 0) {
        buf = malloc((dwLen + 1) * sizeof(_TCHAR));

        /* resolve path from handle */
        if (buf && GetFinalPathNameByHandle(hPath, buf, dwLen+1, dwFlags) > 0) {
            buf[dwLen] = _T('\0');
            CloseHandle(hPath);
            return buf;
        }
    }

    CloseHandle(hPath);
    if (buf) free(buf);

    return NULL;
}

/**
 * Result must be deallocated with free().
 */
_TCHAR *getCanonicalPath(const _TCHAR *lpFileName)
{
    _TCHAR *pathBuf, *linkBuf;

    pathBuf = __getCanonicalPath(lpFileName);
    if (pathBuf) return pathBuf;

    /* __getCanonicalPath() has failed.
     * Treat lpFileName as a symbolic link
     * and try to get its target. */
    linkBuf = getLinkTarget(lpFileName);

    /* lpFileName is not a symbolic link, does not exist,
     * or some other error had occurred. */
    if (!linkBuf) return NULL;

    /* Try again with the link target.
     * This kind of second attempt seems to be required
     * when trying to canonicalize AppExec links. */
    pathBuf = __getCanonicalPath(linkBuf);
    free(linkBuf);

    return pathBuf;
}
