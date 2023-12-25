#include "convert.c"
#include "createLink.c"
#include "getLinkTarget.c"
#include "isSymlink.c"
#include "getCanonicalPath.c"

#define TEST(x)  puts((x) ? "success" : "failure")

int main()
{
    const char *lnk = "link_to_C";

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

    puts("test isSymlinkA");
    TEST(isSymlinkA(lnk) == TRUE);

    return 0;
}
