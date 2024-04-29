#include "convert.c"
#include "createLink.c"
#include "getLinkTarget.c"
#include "isSymlink.c"
#include "getCanonicalPath.c"

#define TEST(x)  puts((x) ? "success" : "failure")

int main()
{
    const char *lnk = "link_to_C";
    const wchar_t *wlnk = L"C:\\Users\\User\\AppData\\Local\\Microsoft\\WindowsApps\\winget.exe";

    DeleteFileA(lnk);
    RemoveDirectoryA(lnk);

    puts("test createLinkA");
    TEST(createLinkA(lnk, "c:/", 'd') == TRUE);
    puts("");

    puts("test getCanonicalPathA");
    char *path = getCanonicalPathA("./././link_to_C");
    TEST(path);

    if (path) {
        puts(path);
        free(path);
    }
    puts("");

    puts("test getLinkTargetA");
    ULONG tag = 0;
    path = getLinkTargetA(lnk, (ULONG *)&tag);
    TEST(path && tag == IO_REPARSE_TAG_SYMLINK);

    if (path) {
        puts(path);
        free(path);
    }
    puts("");

    wprintf(L"test getLinkTargetW [%s]\n", wlnk);
    tag = 0;
    wchar_t *wpath = getLinkTargetW(wlnk, (ULONG *)&tag);
    TEST(wpath && tag == IO_REPARSE_TAG_APPEXECLINK);

    if (wpath) {
        wprintf(L"%s\n", wpath);
        free(wpath);
    }
    puts("");

    puts("test isSymlinkA");
    TEST(isSymlinkA(lnk) == TRUE);

    return 0;
}
