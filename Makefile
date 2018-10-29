CFLAGS=-g
CC=gcc

PROG=update-cache show-diff init-db write-tree read-tree commit-tree cat-file

all: $(PROG)

install: $(PROG)
	install $(PROG) $(HOME)/bin/

LIBS=-lz -lcrypto

init-db: init-db.o xplat_linux.o
	$(CC) $(CFLAGS) -o init-db init-db.o xplat_linux.o $(LIBS)

update-cache: update-cache.o read-cache.o xplat_linux.o
	$(CC) $(CFLAGS) -o update-cache update-cache.o read-cache.o xplat_linux.o $(LIBS)

show-diff: show-diff.o read-cache.o xplat_linux.o
	$(CC) $(CFLAGS) -o show-diff show-diff.o read-cache.o xplat_linux.o $(LIBS)

write-tree: write-tree.o read-cache.o xplat_linux.o
	$(CC) $(CFLAGS) -o write-tree write-tree.o read-cache.o xplat_linux.o $(LIBS)

read-tree: read-tree.o read-cache.o xplat_linux.o
	$(CC) $(CFLAGS) -o read-tree read-tree.o read-cache.o xplat_linux.o $(LIBS)

commit-tree: commit-tree.o read-cache.o xplat_linux.o
	$(CC) $(CFLAGS) -o commit-tree commit-tree.o read-cache.o xplat_linux.o $(LIBS)

cat-file: cat-file.o read-cache.o xplat_linux.o
	$(CC) $(CFLAGS) -o cat-file cat-file.o read-cache.o xplat_linux.o $(LIBS)

read-cache.o: cache.h
show-diff.o: cache.h
xplat_linux.o: xplat.h

clean:
	rm -f *.o $(PROG) temp_git_file_*

backup: clean
	cd .. ; tar czvf dircache.tar.gz dir-cache
