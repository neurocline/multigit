// xplat_windows.c

#include "xplat.h"

#ifdef _WIN32

#include <fcntl.h>
#include <sys/stat.h>

// ----------------------------------------------------------
// Replacements for Linux functions

// xplat_access_R_OK is a Linux access variant hard-wired to mode=R_OK
bool xplat_access_R_OK(const char *pathname)
{
    pathname;
	return -1;
}

// xplat_close is a Linux close clone
int xplat_close(int fd)
{
    fd;
	return -1;
}

// xplat_mkdir is a Linux mkdir clone
int xplat_mkdir(const char *pathname, int mode)
{
    pathname; mode;
	return -1;
}

// xplat_mmap is a Linux mmap variant hardwired to prot=PROT_READ, flags=MAP_PRIVATE
// See http://man7.org/linux/man-pages/man2/mmap.2.html
void *xplat_mmap_READ_PRIVATE(void *addr, size_t length, int fd, off_t offset)
{
    addr; length; fd; offset;
	return 0;
}

int xplat_munmap(void *addr, size_t length)
{
    addr; length;
	return -1;
}

// xplat_mkstemp is a Linux mkstemp clone
int xplat_mkstemp(char* template)
{
    template;
	return -1;
}

// xplat_open is a Linux open clone
// See http://man7.org/linux/man-pages/man2/open.2.html
// This uses varargs to make parameter three optional
//    open(const char *pathname, int flags, mode_t mode)
int xplat_open(const char *pathname, int flags, ...)
{
    pathname; flags;
	// If we have O_CREAT or O_TMPFILE, then we have the
	// three-argument version
	// GRR O_TMPFILE is not available on some systems?
	//if ((flags & (O_CREAT | O_TMPFILE)) != 0)
	if ((flags & O_CREAT) != 0)
	{
		int mode = 0;
		va_list args;
		va_start(args, flags);
		mode = va_arg(args, int);
		va_end(args);
		return -1;
	}
	// Otherwise, we have the two-argument version
	else
		return -1;
}

// xplat_popen is a Linux popen clone
// See http://man7.org/linux/man-pages/man3/popen.3.html
FILE *xplat_popen(const char *command, const char *type)
{
    command; type;
	return 0;
}

// xplat_pclose is a Linux pclose clone
int xplat_pclose(FILE *stream)
{
    stream;
	return -1;
}

static void stat_to_xplat(struct stat* s, struct xplat_stat* statbuf)
{
    s; statbuf;
}

int xplat_stat(const char *pathname, struct xplat_stat *statbuf)
{
    pathname; statbuf;
	return -1;
}

int xplat_fstat(int fd, struct xplat_stat *statbuf)
{
    fd; statbuf;
	return -1;
}

// xplat_unlink is a Linux unlink clone
int xplat_unlink(const char *pathname)
{
    pathname;
	return -1;
}

// xplat_write is a Linux write clone
unsigned long xplat_write(int fd, const void* buf, size_t count)
{
    fd; buf; count;
	return 0UL;
}

// ----------------------------------------------------------
// Cross-platform helper functions

// Return 1 if pathname exists and is a directory, or 0 if not
// For stat, see http://man7.org/linux/man-pages/man2/stat.2.html
bool xplat_is_dir(const char* pathname)
{
    pathname;
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
    info;
	return 1;
}

#endif // _WIN32
