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
#ifndef W32_SYMLINK_REPARSE_DATA_BUFFER_H_INCLUDED
#define W32_SYMLINK_REPARSE_DATA_BUFFER_H_INCLUDED

#include <inttypes.h>
#include <wchar.h>
#include <windows.h>


#ifndef NFS_SPECFILE_LNK
#define NFS_SPECFILE_LNK (0x00000000014B4E4C)
#endif


/* https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_reparse_data_buffer */
typedef struct {
  ULONG  ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  UCHAR  DataBuffer[1];
} REPARSE_DATA_BUFFER;


/* Symbolic links */
typedef struct {
  USHORT SubstituteNameOffset;
  USHORT SubstituteNameLength;
  USHORT PrintNameOffset;
  USHORT PrintNameLength;
  ULONG  Flags;
  WCHAR  PathBuffer[1];
} SYMLINK_REPARSE_BUFFER;


/* Junction points */
typedef struct {
  USHORT SubstituteNameOffset;
  USHORT SubstituteNameLength;
  USHORT PrintNameOffset;
  USHORT PrintNameLength;
  WCHAR  PathBuffer[1];
} MOUNTPOINT_REPARSE_BUFFER;


/* AppExec links (execution aliases) */
/* (those you can find inside C:\Users\<Username>\AppData\Local\Microsoft\WindowsApps) */
/* https://www.tiraniddo.dev/2019/09/overview-of-windows-execution-aliases.html */
/* https://github.com/libuv/libuv/blob/a5c01d4de3695e9d9da34cfd643b5ff0ba582ea7/src/win/winapi.h#L4155 */
typedef struct {
  uint32_t Id;  /* must be 3; this is apparently either a version number or string count */
  wchar_t  StringList[1];
} APPXLINK_REPARSE_BUFFER;


/* Symbolic links from the LinuX SubSystem (UTF-8 formatted, no trailing NUL) */
/* https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-fscc/68337353-9153-4ee1-ac6b-419839c3b7ad */
/* https://github.com/JFLarvoire/SysToolsLib/blob/829ecca8d95ade20f5e6241e1226d27e1a2443e7/C/MsvcLibX/include/reparsept.h#L286 */
typedef struct {
  uint32_t Version;  /* must be 2 */
  char     PathBuffer[1];
} LXSS_SYMLINK_REPARSE_BUFFER;


/* Network File System (NFS) */
/* https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-fscc/ff4df658-7f27-476a-8025-4074c0121eec */
typedef struct {
  uint64_t Type;
  uint8_t  DataBuffer[1];
} NFS_REPARSE_BUFFER;


#endif /* W32_SYMLINK_REPARSE_DATA_BUFFER_H_INCLUDED */
