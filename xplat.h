// xplat.h
// Simple cross-platform layer

#pragma once

// C99 doesn't have bool, it has _Bool, for legacy reasons.
// But <stdbool.h> creates a bool type, and defines true and false.
#include <stdbool.h>

#include <stddef.h> // for size_t
#include <stdio.h> // for FILE*

#include <zlib.h>

// Use built-in SHA-1 always
#include "block-sha1/sha1.h"

#define SHA_CTX blk_SHA_CTX
#define SHA1_Init blk_SHA1_Init
#define SHA1_Update blk_SHA1_Update
#define SHA1_Final blk_SHA1_Final

// ---------------------------------------------------------------------------

// Cross-platform versions of Linux functions
// This won't last long, I think, it's just an easy next step.

// xplat_access_R_OK is a Linux access variant hard-wired to mode=R_OK
// See http://man7.org/linux/man-pages/man2/access.2.html
bool xplat_access_R_OK(const char *pathname);

// xplat_close is a Linux close clone
// See http://man7.org/linux/man-pages/man2/close.2.html
int xplat_close(int fd);

// xplat_mkdir is a Linux mkdir clone
// See http://man7.org/linux/man-pages/man2/mkdir.2.html
// See http://pubs.opengroup.org/onlinepubs/009696699/basedefs/sys/types.h.html
// See http://pubs.opengroup.org/onlinepubs/7908799/xsh/sysstat.h.html
int xplat_mkdir(const char *pathname, int mode);

// xplat_mmap is a Linux mmap variant hardwired to prot=PROT_READ, flags=MAP_PRIVATE
// xplat_munmap is a Linux munmap clone
// See http://man7.org/linux/man-pages/man2/mmap.2.html
void *xplat_mmap_READ_PRIVATE(void *addr, size_t length, int fd, off_t offset);
int xplat_munmap(void *addr, size_t length);

// xplat_mkstemp is a Linux mkstemp clone
// See http://man7.org/linux/man-pages/man3/mkstemp.3.html
int xplat_mkstemp(char* template);

// xplat_open is a Linux open clone
// See http://man7.org/linux/man-pages/man2/open.2.html
int xplat_open(const char *pathname, int flags, ...);

// xplat_popen is a Linux popen clone
// xplat_pclose is a Linux pclose clone
// See http://man7.org/linux/man-pages/man3/popen.3.html
FILE *xplat_popen(const char *command, const char *type);
int xplat_pclose(FILE *stream);

// xplat_stat is a Linux stat clone
// xplat_fstat is a Linux fstat clone
// See http://man7.org/linux/man-pages/man2/stat.2.html
// See https://linux.die.net/man/3/clock_gettime
struct xplat_timespec {
	long   tv_sec;        /* seconds */
	long   tv_nsec;       /* nanoseconds */
};

struct xplat_stat {
	int     st_dev;         /* ID of device containing file */
	int     st_ino;         /* Inode number */
	int     st_mode;        /* File type and mode */
	int     st_nlink;       /* Number of hard links */
	int     st_uid;         /* User ID of owner */
	int      st_gid;         /* Group ID of owner */
	int      st_rdev;        /* Device ID (if special file) */
	long long st_size;        /* Total size, in bytes */
	long     st_blksize;     /* Block size for filesystem I/O */
	int      st_blocks;      /* Number of 512B blocks allocated */

	/* Since Linux 2.6, the kernel supports nanosecond
	  precision for the following timestamp fields.
	  For the details before Linux 2.6, see NOTES. */

	struct xplat_timespec st_atim;  /* Time of last access */
	struct xplat_timespec st_mtim;  /* Time of last modification */
	struct xplat_timespec st_ctim;  /* Time of last status change */
};
int xplat_stat(const char *pathname, struct xplat_stat *statbuf);
int xplat_fstat(int fd, struct xplat_stat *statbuf);

// xplat_unlink is a Linux unlink clone
// See http://man7.org/linux/man-pages/man2/unlink.2.html
int xplat_unlink(const char *pathname);

// xplat_write is a Linux write clone
// See http://man7.org/linux/man-pages/man2/write.2.html
unsigned long xplat_write(int fd, const void* buf, size_t count);

// ---------------------------------------------------------------------------

// xplat_is_dir returns true if pathname points to an existing directory
bool xplat_is_dir(const char* pathname);

// Struct containing bits of commit info that are fetched in a machine-specific way
struct commit_info
{
    char* gecos;
    char* email;
    char* date;
    char* realgecos;
    char realemail[1000];
    char* realdate;
    char comment[1000];
};

int fill_commit_info(struct commit_info* info);
