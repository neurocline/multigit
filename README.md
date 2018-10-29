# Cross-platform Git

This is the original mainline Git code, updated so that it is
cross-platform and up to date in terms of C compiler (no GNU
extensions). It is cross-platform in the senses that it builds
for Linux, Windows and Mac OS X.

It also uses Premake for the makefile, because Unix make isn't really
available on Windows, and nmake isn't all that close to make.

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
$ git reset --hard
... remove files we don't need ...
$ git checkout cgit -- .
$ git checkout master -- .gitignore .gitattributes .editorconfig
... edit ...
$ git commit ...
```

and now we have the first commit.

The first thing we do is add Premake binaries and premake5.lua makefile. We need
some sort of cross-platform make, and it's either CMake or Premake5. For now,
use Premake5 because it's slightly easier to get started.
