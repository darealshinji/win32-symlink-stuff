#include "createLink.c"
#include "getCanonicalPath.c"
#include "getLinkTarget.c"
#include "isSymlink.c"

#define TEST(x)  puts((x) ? "success" : "failure")

int main()
{
    const char *lnk = "link_to_C";

    DeleteFileA(lnk);
    RemoveDirectoryA(lnk);

    puts("test createLink");
    TEST(createLink(lnk, "c:/", 'd') == TRUE);
    puts("");

    puts("test getCanonicalPath");
    char *path = getCanonicalPath("./././link_to_C");
    TEST(path);

    if (path) {
        puts(path);
        free(path);
    }
    puts("");

    puts("test getLinkTarget");
    ULONG tag = 0;
    path = getLinkTarget(lnk, (ULONG *)&tag);
    TEST(path && tag == IO_REPARSE_TAG_SYMLINK);

    if (path) {
        puts(path);
        free(path);
    }
    puts("");

    puts("test isSymlink");
    TEST(isSymlink(lnk) == TRUE);

    return 0;
}
