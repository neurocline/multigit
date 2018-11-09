CFLAGS=-g
CC=gcc

PROG=update-cache show-diff init-db write-tree read-tree commit-tree cat-file

all: $(PROG)

install: $(PROG)
	install $(PROG) $(HOME)/bin/

# Set up libraries
# - Define BLK_SHA1 to use internal block_sha1 instead of libcrypto
# - Define BUILTIN_ZLIB to use internal zlib instead of system zlib

LIBS = read-cache.o
ifdef BLK_SHA1
	LIBS += block-sha1/sha1.o
	CFLAGS += -DSHA1_BLK
else
	LIBS += -lcrypto
endif
ifdef BUILTIN_ZLIB
	LIBS += zlib/lib.a
	CFLAGS += -DBUILTIN_ZLIB
else
	LIBS += -lz
endif

init-db: init-db.o

update-cache: update-cache.o $(LIBS)
	$(CC) $(CFLAGS) -o update-cache update-cache.o $(LIBS)

show-diff: show-diff.o $(LIBS)
	$(CC) $(CFLAGS) -o show-diff show-diff.o $(LIBS)

write-tree: write-tree.o $(LIBS)
	$(CC) $(CFLAGS) -o write-tree write-tree.o $(LIBS)

read-tree: read-tree.o $(LIBS)
	$(CC) $(CFLAGS) -o read-tree read-tree.o  $(LIBS)

commit-tree: commit-tree.o $(LIBS)
	$(CC) $(CFLAGS) -o commit-tree commit-tree.o $(LIBS)

cat-file: cat-file.o $(LIBS)
	$(CC) $(CFLAGS) -o cat-file cat-file.o $(LIBS)

read-cache.o: cache.h
show-diff.o: cache.h

ZLIB_C := $(wildcard zlib/*.c)
ZLIB_O := $(subst .c,.o,$(ZLIB_C))

$(ZLIB_O): %.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

zlib/lib.a: $(ZLIB_O)
	$(AR) $(ARFLAGS) $@ $^

block_sha1/sha1.o: block_sha1/sha1.c

clean:
	rm -f *.o $(PROG) temp_git_file_*
	rm -f zlib/*.o zlib/lib.a block-sha1/*.o

backup: clean
	cd .. ; tar czvf dircache.tar.gz dir-cache
