# Multigit

Implementation of Git in multiple languages

- branch `cgit` is mainline Git in C (from the github.com/git/git mirror)
- branch `cpcgit` is mainline Git in C, made cross-platform
- branch `gogit` is a literal revision-for-revision translation into Go

The master branch contains submodule merges from each language branch,
in a directory that matches the branch name.

## Creating cgit

We import the code we are working on from the mainline Git repo. This
is mirrored on Github at `github.com/git/git`. For now, all we want
is the first commit. We'll add more commits as we make progress.

We remove the mainline remote as soon as we are done, so that it doesn't
clutter up repo visualization (gitk or other graphical tool). We will add
it back when we need more commits.

```
$ git remote add mainline git@github.com:git/git.git
$ git fetch mainline
$ git branch cgit 8c91cbcb8dd5c12ef24b5f35e4fdcc3780568d90
$ git push -u origin cgit
$ git remote remove mainline.
```

## Creating cpcgit

We have the relevant code in `cgit`, so we just mirror it from that
branch into a new branch. We also add metadata files from `master`.

```
$ git checkout --orphan cpcgit
$ git rm --cached -r .
$ git reset --hard
$ git checkout cgit -- .
$ git checkout master -- .gitignore .gitattributes .editorconfig
... do work ...
$ git commit ...
$ git push -u origin cpcgit
```

Note that `git --orphan && git rm --cached -r .` removes files from Git's knowledge,
but does not delete files in the working directory. If you want to wipe them and you have
no important files hanging around, use `git clean -f` (use `--dry-run` as a first tentative
step to make sure). This will not remove ignored files, so set up your `.gitignore` properly.

This puts new copies of files in the working directory and the index, so that
we can commit them fresh to a new branch.

Also note that until you commit something, there's not really a branch.

## Creating gogit

We just want a new empty branch, albeit with metadata files.

```
$ git checkout --orphan cpcgit
$ git rm --cached -r .
$ git reset --hard
$ git checkout master -- .gitignore .gitattributes .editorconfig
... do work ...
$ git commit ...
$ git push -u origin gogit
```
