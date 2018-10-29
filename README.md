# Cross-platform Git

This is the original mainline Git code, updated so that it is
cross-platform and up to date in terms of C compiler (no GNU
extensions). It is cross-platform in the senses that it builds
for Linux, Windows and Mac OS X.

It also uses Premake for the makefile, because Unix `make` isn't really
available on Windows, and `nmake` isn't all that close to `make`.

## Notes

The code now assumes a C99 compiler. This is a reasonable assumption.
That means we can use single-line `//` comments as well as multiline `/*...*/` comments.
This also makes it easy to see new comments, they are in single-line style.

The code also expects the compiler to support `#pragma once`.

## How the code was made portable

Our pure source code is in the `cgit` branch, at commit `8c91cbcb8dd5c12ef24b5f35e4fdcc3780568d90`
(which we imported from the mainline Git repo). Make a copy of this into
a new branch, and get typical repo metadata files as well.

```
$ git checkout --orphan cpcgit
$ git rm --cached -r .
... remove files we don't need ...
$ git checkout cgit -- .
$ git checkout master -- .gitignore .gitattributes .editorconfig
... edit ...
```

### Premake

The first thing we do is add Premake binaries and premake5.lua makefile. We need
some sort of cross-platform make, and it's either CMake or Premake5. For now,
use Premake5 because it's slightly easier to get started.

```
$ mkdir bin
$ subl README.md
$ ...
$ ... download Premak5 binaries
```

We write a basic premake5.lua:

```
-- premake5.lua
workspace "cpcgit"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location("build")
    language "C++"
    cppdialect "C++17"
    warnings "Extra"

    filter { "platforms:*32" }
        architecture "x86"
    filter { "platforms:*64" }
        architecture "x64"
    filter { "toolset:msc*" }
        defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
    filter { "action:xcode* or toolset:clang*" }
        buildoptions { "-mlzcnt" }

project "cpcgit"
    location "build"
    kind "ConsoleApp"
    files { "*.c", "*.h" }
```

### Get code compiling on modern Linux

As-is, the code doesn't build on a 2018-era Linux like Ubuntu. Fix that.

- cache.h: `memcmp` now needs header file `<string.h>`
- Makefile: `LIBS` needs `-lz` to link Zlib
- Makefile: `LIBS` needs `-lcrypto` to link SHA1 routines from libcrypto
- init-db.c: the first return needs to be `return 0`
- read-cache.d: fix bug with empty index

Once we make those fixes, the code compiles and links.

The diff:

```
diff -u ../git-original/cache.h ./cache.h
--- ../git-original/cache.h   2018-10-26 19:19:45.753511000 -0700
+++ ./cache.h   2018-10-29 08:04:04.710802800 -0700
@@ -9,6 +9,7 @@
 #include <stdarg.h>
 #include <errno.h>
 #include <sys/mman.h>
+#include <string.h>

 #include <openssl/sha.h>
 #include <zlib.h>
diff -u ../git-original/init-db.c ./init-db.c
--- ../git-original/init-db.c 2018-10-26 19:19:45.755481900 -0700
+++ ./init-db.c 2018-10-29 08:06:28.934389500 -0700
@@ -20,12 +20,12 @@
        if (sha1_dir) {
                struct stat st;
                if (!stat(sha1_dir, &st) < 0 && S_ISDIR(st.st_mode))
-                       return;
+                       return 0;
                fprintf(stderr, "DB_ENVIRONMENT set to bad directory %s: ", sha1_dir);
        }

        /*
-        * The default case is to have a DB per managed directory.
+        * The default case is to have a DB per managed directory.
         */
        sha1_dir = DEFAULT_DB_ENVIRONMENT;
        fprintf(stderr, "defaulting to private storage area\n");
diff -u ../git-original/Makefile ./Makefile
--- ../git-original/Makefile  2018-10-26 19:19:45.751516200 -0700
+++ ./Makefile  2018-10-29 08:05:39.629685200 -0700
@@ -8,7 +8,7 @@
 install: $(PROG)
        install $(PROG) $(HOME)/bin/

-LIBS= -lssl
+LIBS= -lssl -lz -lcrypto

 init-db: init-db.o

diff -u ../git-original/read-cache.c ./read-cache.c
--- ../git-original/read-cache.c      2018-10-26 19:19:45.755481900 -0700
+++ ./read-cache.c      2018-10-29 08:10:12.419215900 -0700
@@ -228,7 +228,7 @@
                map = NULL;
                size = st.st_size;
                errno = EINVAL;
-               if (size > sizeof(struct cache_header))
+               if (size >= sizeof(struct cache_header))
                        map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        }
        close(fd);
```
