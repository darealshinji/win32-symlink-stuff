/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2023-2025 Carsten Janssen
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
#include <wchar.h>
#include "convert.h"
#include "reparse_data_buffer.h"
#include "w32-symlink.h"


int isSymlinkW(const wchar_t *path, ULONG *tag)
{
    UINT8 data[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
    REPARSE_DATA_BUFFER *pData;
    NFS_LNK_REPARSE_BUFFER *pNfs;
    HANDLE handle;
    DWORD dwAttr;

    dwAttr = GetFileAttributesW(path);

    if (dwAttr == INVALID_FILE_ATTRIBUTES) {
        /* error */
        return -1;
    }

    if (!(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT)) {
        /* not a symbolic link */
        return FALSE;
    }

    if (!tag) {
        /* a symbolic link but we don't
         * need to know what kind of symlink */
        return TRUE;
    }

    /* open path for reading */
    handle = CreateFileW(path,
                         0,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                         NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        return -1;
    }

    /* retrieve reparse data */
    if (!DeviceIoControl(handle,
                         FSCTL_GET_REPARSE_POINT,
                         NULL,
                         0,
                         data,
                         MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
                         NULL,
                         NULL))
    {
        CloseHandle(handle);
        return -1;
    }

    CloseHandle(handle);

    pData = (REPARSE_DATA_BUFFER *)data;
    *tag = pData->ReparseTag;

    switch (*tag)
    {
    case IO_REPARSE_TAG_SYMLINK:
    case IO_REPARSE_TAG_MOUNT_POINT:
    case IO_REPARSE_TAG_APPEXECLINK:
    case IO_REPARSE_TAG_LX_SYMLINK:
        return TRUE;

    case IO_REPARSE_TAG_NFS:
        pNfs = (NFS_LNK_REPARSE_BUFFER *)pData->DataBuffer;
        return (pNfs->Type == NFS_SPECFILE_LNK) ? TRUE : FALSE;

    default:
        break;
    }

    return FALSE;
}


int isSymlinkA(const char *path, ULONG *tag)
{
    int rv;
    wchar_t *wstr;

    wstr = convert_str_to_wcs(path);
    if (!wstr) return -1;

    rv = isSymlinkW(wstr, tag);
    free(wstr);

    return rv;
}
