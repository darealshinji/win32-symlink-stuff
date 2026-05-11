CFLAGS    = -Wall -Wextra -O3 -Iinclude -Isource
CFLAGS   += -Werror=implicit-function-declaration
CPPFLAGS  = -DWIN32_LEAN_AND_MEAN
#CPPFLAGS += -DUTF8_EVERYWHERE
LDFLAGS   = -s

# ls -1 source/*.c source/posix/*.c | sed 's,^,\t,; s,\.c$,.o \\,'
OBJS = \
	source/common_a.o \
	source/common_w.o \
	source/convert.o \
	source/createLinkA.o \
	source/createLinkW.o \
	source/getCanonicalPathA.o \
	source/getCanonicalPathW.o \
	source/getLinkTargetA.o \
	source/getLinkTargetW.o \
	source/isSymlinkA.o \
	source/isSymlinkW.o \
	source/posix/link_a.o \
	source/posix/link_w.o \
	source/posix/lstat.o \
	source/posix/lstat64_a.o \
	source/posix/lstat64_w.o \
	source/posix/oldnames.o \
	source/posix/readlink_a.o \
	source/posix/readlink_w.o \
	source/posix/realpath_a.o \
	source/posix/realpath_w.o \
	source/posix/symlink_a.o \
	source/posix/symlink_w.o \
	$(NULL)

ARCHIVE = symlink.a
TEST_FILES = test/test1.exe test/test2.exe test/test3.exe


all: $(ARCHIVE)

tests: $(TEST_FILES)

clean:
	-rm -f *.a test/*.exe test/*.o source/*.o source/posix/*.o

$(ARCHIVE): $(OBJS)
	$(AR) crs $@ $(OBJS)

test/test1.exe: test/test1.o $(ARCHIVE)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

test/test2.exe: test/test2.o $(ARCHIVE)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

test/test3.exe: test/test3.o $(ARCHIVE)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
