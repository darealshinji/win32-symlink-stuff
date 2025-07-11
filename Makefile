CFLAGS = -Wall -Wextra -O3 -Iinclude
LDFLAGS = -s

OBJS = source/convert.o \
	source/createLink.o \
	source/getCanonicalPath.o \
	source/getLinkTarget.o \
	source/isSymlink.o \
	source/lstat.o \
	source/posix.o

ARCHIVE = symlink.a
TEST_FILES = test/test1.exe test/test2.exe test/test3.exe


all: $(ARCHIVE)

tests: $(TEST_FILES)

clean:
	-rm -f *.a test/*.exe test/*.o source/*.o

$(ARCHIVE): $(OBJS)
	$(AR) crs $@ $(OBJS)

test/test1.exe: test/test1.o $(ARCHIVE)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

test/test2.exe: test/test2.o $(ARCHIVE)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

test/test3.exe: test/test3.o $(ARCHIVE)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
