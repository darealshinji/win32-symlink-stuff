/**
 * The MIT License (MIT)
 *
 * Copyright (C) 2023-2026 Carsten Janssen
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
#ifndef W32_SYMLINK_WIN32_H_INCLUDED
#define W32_SYMLINK_WIN32_H_INCLUDED

#include <windows.h>
#include <wchar.h>


/**
 * #define UTF8_EVERYWHERE to assume UTF-8 encoding in "char" strings, otherwise
 * strings will be converted to "wchar_t" and the *W API functions will be used.
 */


#ifdef __cplusplus
extern "C" {
#endif



#undef __WARN_UNUSED_RESULT
#ifdef __GNUC__
#define __WARN_UNUSED_RESULT  __attribute__((warn_unused_result))
#else
#define __WARN_UNUSED_RESULT  /**/
#endif



/* https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-fscc/c8e77b37-3909-4fe6-a4ea-2b9d423b1ee4 */
#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK      (0xA000000C)
#endif
#ifndef IO_REPARSE_TAG_MOUNT_POINT
#define IO_REPARSE_TAG_MOUNT_POINT  (0xA0000003)
#endif
#ifndef IO_REPARSE_TAG_NFS
#define IO_REPARSE_TAG_NFS          (0x80000014)
#endif
#ifndef IO_REPARSE_TAG_APPEXECLINK
#define IO_REPARSE_TAG_APPEXECLINK  (0x8000001B)
#endif
#ifndef IO_REPARSE_TAG_LX_SYMLINK
#define IO_REPARSE_TAG_LX_SYMLINK   (0xA000001D)
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

#ifdef _UNICODE
#define createLink createLinkW
#else
#define createLink createLinkA
#endif

BOOL createLinkA(const char *lpLinkName, const char *lpTargetName, char mode);
BOOL createLinkW(const wchar_t *lpLinkName, const wchar_t *lpTargetName, char mode);



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

#ifdef _UNICODE
#define getCanonicalPath getCanonicalPathW
#else
#define getCanonicalPath getCanonicalPathA
#endif

char    *getCanonicalPathA(const char *lpFileName) __WARN_UNUSED_RESULT;
wchar_t *getCanonicalPathW(const wchar_t *lpFileName) __WARN_UNUSED_RESULT;



/**
 * getLinkTarget() will return an allocated string with the link's target.
 * This function is similar to POSIX's `readlink(2)`.
 *
 * pReparseTag is a pointer to the variable where the Reparse Tag will be
 * saved if reading the reparse data was successful. This can be used to
 * figure out what type of link this is. This parameter can be set NULL.
 *
 * The result must be deallocated with free().
 */

#ifdef _UNICODE
#define getLinkTarget getLinkTargetW
#else
#define getLinkTarget getLinkTargetA
#endif

char    *getLinkTargetA(const char *lpFileName, ULONG *pReparseTag) __WARN_UNUSED_RESULT;
wchar_t *getLinkTargetW(const wchar_t *lpFileName, ULONG *pReparseTag) __WARN_UNUSED_RESULT;



/**
 * Whether lpFileName is a symbolic link or not.
 *
 * pReparseTag is a pointer to the variable where the Reparse Tag will be
 * saved. This can be used to figure out what type of link this is.
 * This parameter can be set NULL.
 *
 * Return values of isSymlink():
 *  1 (TRUE)    lpFileName is a symbolic link
 *  0 (FALSE)   lpFileName exists and is NOT a symbolic link
 * -1           an error has occured (i.e. if lpFileName does not exist)
 */

#ifdef _UNICODE
#define isSymlink isSymlinkW
#else
#define isSymlink isSymlinkA
#endif

int isSymlinkA(const char *lpFileName, ULONG *pReparseTag);
int isSymlinkW(const wchar_t *lpFileName, ULONG *pReparseTag);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* W32_SYMLINK_WIN32_H_INCLUDED */
