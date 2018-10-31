// xplat_linux.cpp
// Simple cross-platform layer

#include "xplat.h"

// Linux-only file
#ifdef __linux__

#include <fcntl.h> // open
#include <pwd.h> // used for getpwuid
#include <stddef.h> // for size_t
#include <stdio.h> // for FILE*
#include <stdlib.h> // for mkstemp
#include <string.h>
#include <sys/mman.h> // for mmap
#include <sys/stat.h> // for mkdir, stat
#include <sys/types.h> // for mkdir, S_ISDIR
#include <time.h>
#include <unistd.h> // for access, write

#include <zlib.h>

// ----------------------------------------------------------
// Replacements for Linux functions

// xplat_access_R_OK is a Linux access variant hard-wired to mode=R_OK
bool xplat_access_R_OK(const char *pathname)
{
	return access(pathname, R_OK) == 0;
}

// xplat_close is a Linux close clone
int xplat_close(int fd)
{
	return close(fd);
}

// xplat_mkdir is a Linux mkdir clone
int xplat_mkdir(const char *pathname, int mode)
{
	return mkdir(pathname, mode);
}

// xplat_mmap is a Linux mmap variant hardwired to prot=PROT_READ, flags=MAP_PRIVATE
// See http://man7.org/linux/man-pages/man2/mmap.2.html
void *xplat_mmap_READ_PRIVATE(void *addr, size_t length, int fd, off_t offset)
{
	int prot = PROT_READ;
	int flags = MAP_PRIVATE;
	return mmap(addr, length, prot, flags, fd, offset);
}

int xplat_munmap(void *addr, size_t length)
{
	return munmap(addr, length);
}

// xplat_mkstemp is a Linux mkstemp clone
int xplat_mkstemp(char* template)
{
	return mkstemp(template);
}

// xplat_open is a Linux open clone
// See http://man7.org/linux/man-pages/man2/open.2.html
// This uses varargs to make parameter three optional
//    open(const char *pathname, int flags, mode_t mode)
int xplat_open(const char *pathname, int flags, ...)
{
	// If we have O_CREAT or O_TMPFILE, then we have the
	// three-argument version
	// GRR O_TMPFILE is not available on some systems?
	//if ((flags & (O_CREAT | O_TMPFILE)) != 0)
	if ((flags & O_CREAT) != 0)
	{
		mode_t mode = 0;
		va_list args;
		va_start(args, flags);
			mode = va_arg(args, mode_t);
		va_end(args);
		return open(pathname, flags, mode);
	}
	// Otherwise, we have the two-argument version
	else
		return open(pathname, flags);
}

// xplat_popen is a Linux popen clone
// See http://man7.org/linux/man-pages/man3/popen.3.html
FILE *xplat_popen(const char *command, const char *type)
{
	return popen(command, type);
}

// xplat_pclose is a Linux pclose clone
int xplat_pclose(FILE *stream)
{
	return pclose(stream);
}

static void stat_to_xplat(struct stat* s, struct xplat_stat* statbuf)
{
	statbuf->st_dev = s->st_dev;
	statbuf->st_ino = s->st_ino;
	statbuf->st_mode = s->st_mode;
	statbuf->st_nlink = s->st_nlink;
	statbuf->st_uid = s->st_uid;
	statbuf->st_gid = s->st_gid;
	statbuf->st_rdev = s->st_rdev;
	statbuf->st_size = s->st_size;
	statbuf->st_blksize = s->st_blksize;
	statbuf->st_blocks = s->st_blocks;
	statbuf->st_atim.tv_sec = s->st_atim.tv_sec;
	statbuf->st_atim.tv_nsec = s->st_atim.tv_nsec;
	statbuf->st_mtim.tv_sec = s->st_mtim.tv_sec;
	statbuf->st_mtim.tv_nsec = s->st_mtim.tv_nsec;
	statbuf->st_ctim.tv_sec = s->st_ctim.tv_sec;
	statbuf->st_ctim.tv_nsec = s->st_ctim.tv_nsec;
}

int xplat_stat(const char *pathname, struct xplat_stat *statbuf)
{
	struct stat s;
	int ret = stat(pathname, &s);
	stat_to_xplat(&s, statbuf);
	return ret;
}

int xplat_fstat(int fd, struct xplat_stat *statbuf)
{
	struct stat s;
	int ret = fstat(fd, &s);
	stat_to_xplat(&s, statbuf);
	return ret;
}

// xplat_unlink is a Linux unlink clone
int xplat_unlink(const char *pathname)
{
	return unlink(pathname);
}

// xplat_write is a Linux write clone
unsigned long xplat_write(int fd, const void* buf, size_t count)
{
	return write(fd, buf, count);
}

// ----------------------------------------------------------
// Cross-platform helper functions

// Return 1 if pathname exists and is a directory, or 0 if not
// For stat, see http://man7.org/linux/man-pages/man2/stat.2.html
bool xplat_is_dir(const char* pathname)
{
	struct stat st;
	if (stat(pathname, &st) == 0 && S_ISDIR(st.st_mode))
		return true;

	// patname not found or exists but is not a directory
	return false;
}

// Used by fill_commit_info to clean up names from OS
static void remove_special(char *p)
{
	char c;
	char *dst = p;

	for (;;) {
		c = *p;
		p++;
		switch(c) {
		case '\n': case '<': case '>':
			continue;
		}
		*dst++ = c;
		if (!c)
			break;
	}
}

int fill_commit_info(struct commit_info* info)
{
    int len;
	struct passwd *pw;
	time_t now;

	pw = getpwuid(getuid());
	if (!pw)
		return 0;
	info->realgecos = pw->pw_gecos;
	len = strlen(pw->pw_name);
	memcpy(info->realemail, pw->pw_name, len);
	info->realemail[len] = '@';
	gethostname(info->realemail+len+1, sizeof(info->realemail)-len-1);
	time(&now);
	info->realdate = ctime(&now);

	info->gecos = getenv("COMMITTER_NAME"); if (!info->gecos) info->gecos = info->realgecos;
	info->email = getenv("COMMITTER_EMAIL"); if (!info->email) info->email = info->realemail;
	info->date = getenv("COMMITTER_DATE"); if (!info->date) info->date = info->realdate;

	remove_special(info->gecos); remove_special(info->realgecos);
	remove_special(info->email); remove_special(info->realemail);
	remove_special(info->date); remove_special(info->realdate);

    return 1;
}

#endif // __linux__
