#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

#include "w32-symlink.h"

#define TEST(x)  puts((x) ? "success" : "failure")


int main()
{
    const wchar_t *nfs_file = L"nfs_file";
    const wchar_t *nfs_link = L"nfs_link";
    wchar_t *wpath;
    ULONG tag = 0;

    wprintf(L"test getLinkTargetW [%s]\n", nfs_link);
    wpath = getLinkTargetW(nfs_link, (ULONG *)&tag);
    TEST(wpath && tag == IO_REPARSE_TAG_NFS);

    if (wpath) {
        wprintf(L"%s\n", wpath);
        free(wpath);
    }
    puts("");

    puts("test isSymlinkW (link)");
    tag = 0;
    TEST(isSymlinkW(nfs_link, (ULONG *)&tag) == TRUE && tag == IO_REPARSE_TAG_NFS);
    puts("");

    puts("test isSymlinkW (file)");
    tag = 0;
    TEST(isSymlinkW(nfs_file, (ULONG *)&tag) == FALSE && tag != IO_REPARSE_TAG_NFS);

    return 0;
}
