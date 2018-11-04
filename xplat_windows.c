// xplat_windows.c

#include "xplat.h"

#ifdef _WIN32

#include <fcntl.h>
#include <sys/stat.h>

#include <malloc.h>

// ----------------------------------------------------------
// Replacements for Linux functions

#define maxFileIds 16
static HANDLE fileIds[maxFileIds];
static int fileId(HANDLE h)
{
	// Find the next open slot
	for (int i = 0; i < maxFileIds; i++)
	{
		if (fileIds[i] == NULL)
		{
			fileIds[i] = h;
			return i + 3; // skip past stdin/out/err just in case
		}
	}
	return -1; // no more room
}

// xplat_access_R_OK is a Linux access variant hard-wired to mode=R_OK
bool xplat_access_R_OK(const char *pathname)
{
	// Just see that we can read file attributes, anything else is too slow
    DWORD attr = GetFileAttributesA(pathname);
    return attr != INVALID_FILE_ATTRIBUTES;
}

// xplat_close is a Linux close clone
int xplat_close(int fd)
{
	if (fd < 3)
		return -1;

    // Get the handle and close it
    HANDLE h = fileIds[fd-3];
	if (h == NULL)
		return -1;
	fileIds[fd-3] = 0;
	CloseHandle(h);
	return 0;
}

// xplat_mkdir is a Linux mkdir clone
int xplat_mkdir(const char *pathname, int mode)
{
	mode; // TBD synthesize LPSECURITY_ATTRIBUTES from mode
    return CreateDirectoryA(pathname, NULL) ? 0 : -1;
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

// xplat_mkstemp is a Linux mkstemp clone, which gets a path
// of the form <path>XXXXXX, where all the X characters at the
// end are turned into something that makes for a unique path.
int xplat_mkstemp(char* template)
{
    char parent[MAX_PATH];
	char temppath[MAX_PATH];
	char prefix[MAX_PATH];
	int size; // size of incoming template
	int num_pat; // number of characters to append to the prefix
	char* pat;
	char* name;
	char leaf[MAX_PATH];

	// If the template is too long for a path, give up
	size = strlen(template);
	if (size >= MAX_PATH)
		return -1;

	// Break into parent + prefix + suffix
	pat = template + size - 1;
	while (pat > template && *pat == 'X')
		pat--;

	// Strip off parent - if this is a relative path, then
	// parent = '.'
	if (template[0] == '/' || template[1] == ':')
	{
		name = strrchr(parent, '/');
		strncpy(parent, template, name - template);
		*name = 0;
	}
	else
	{
		name = template;
		strcpy(parent, ".");
	}

	// Get the prefix
	*pat = 0;
	strncpy(prefix, name+1, pat-name);

	// The user wants size-pat characters added to the
	// prefix. If num_pat > 4, then we'll need to generate
	// more prefix on our own, because we are going to generate
	// a random number in the range 1000-9999.
	num_pat = template + size - pat;

	// Start with 1000
	// TBD - ditch GetTempFileNameA and do it all ourselves
	UINT uUnique = 1000;
	UINT result = GetTempFileNameA(parent, leaf, uUnique, temppath);
	if (result == 0)
		return -1;

	// This is not correct, but it's good enough for now
	strncpy(template, temppath, size);

	// Now open it
	return xplat_open(template, O_RDWR|O_CREAT|O_EXCL, /*S_IRUSR|S_IWUSR*/ 0700);
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
	int mode = 0;
	DWORD dispose = OPEN_EXISTING;
	if ((flags & O_CREAT) != 0)
	{
		dispose = CREATE_NEW;
		va_list args;
		va_start(args, flags);
		mode = va_arg(args, int);
		va_end(args);
		// and then just ignore it on Windows qq
	}

	DWORD access = GENERIC_READ|GENERIC_WRITE;
	DWORD share = FILE_SHARE_READ|FILE_SHARE_WRITE;
	DWORD createflags = FILE_ATTRIBUTE_NORMAL;
	HANDLE h = CreateFileA(pathname, access, share, NULL, dispose, createflags, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return -1;
	return fileId(h);
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

// The Windows epoch starts at 1601-01-01T00:00:00Z, which is 11644473600
// before the POSIX epoch 1970-01-01T00:00:00Z. Windows ticks are in 100 nanoseconds.
static void posixTimestamp(FILETIME* ft, struct xplat_timespec* posix)
{
	unsigned long long windowsTicks = ((unsigned long long)ft->dwHighDateTime << 32ULL) + ft->dwLowDateTime;
	posix->tv_sec = (long)(windowsTicks / 10000000 - 11644473600LL);
	posix->tv_nsec = (long)(windowsTicks * 100 - posix->tv_sec);
}

static void stat_to_xplat(WIN32_FILE_ATTRIBUTE_DATA* s, struct xplat_stat* statbuf)
{
	statbuf->st_dev = 0;
	statbuf->st_ino = 0;
	statbuf->st_mode = 0;
	statbuf->st_nlink = 0;
	statbuf->st_uid = 0;
	statbuf->st_gid = 0;
	statbuf->st_rdev = 0;
	statbuf->st_size = (long long)(((unsigned long long)s->nFileSizeHigh << 32LL) + s->nFileSizeLow);
	statbuf->st_blksize = 512;
	statbuf->st_blocks = (int)(statbuf->st_size + 511) / 512;
	posixTimestamp(&s->ftLastAccessTime, &statbuf->st_atim);
	posixTimestamp(&s->ftLastWriteTime, &statbuf->st_mtim);
	posixTimestamp(&s->ftCreationTime, &statbuf->st_ctim);
}

static void findinfo_to_xplat(WIN32_FIND_DATAA* info, struct xplat_stat* statbuf)
{
	statbuf->st_dev = 0;
	statbuf->st_ino = 0;
	statbuf->st_mode = info->dwFileAttributes;
	statbuf->st_nlink = 0;
	statbuf->st_uid = 0;
	statbuf->st_gid = 0;
	statbuf->st_rdev = 0;
	statbuf->st_size = (long long)(((unsigned long long)info->nFileSizeHigh << 32LL) + info->nFileSizeLow);
	statbuf->st_blksize = 512;
	statbuf->st_blocks = (int)(statbuf->st_size + 511) / 512;
	posixTimestamp(&info->ftLastAccessTime, &statbuf->st_atim);
	posixTimestamp(&info->ftLastWriteTime, &statbuf->st_mtim);
	posixTimestamp(&info->ftCreationTime, &statbuf->st_ctim);
}

static void fileinfo_to_xplat(BY_HANDLE_FILE_INFORMATION* info, struct xplat_stat* statbuf)
{
	statbuf->st_dev = 0;
	statbuf->st_ino = info->nFileIndexLow; // this needs to be 64-bit to be anything other than "changed"
	statbuf->st_mode = info->dwFileAttributes;
	statbuf->st_nlink = info->nNumberOfLinks;
	statbuf->st_uid = 0;
	statbuf->st_gid = 0;
	statbuf->st_rdev = 0;
	statbuf->st_size = (long long)(((unsigned long long)info->nFileSizeHigh << 32LL) + info->nFileSizeLow);
	statbuf->st_blksize = 512;
	statbuf->st_blocks = (int)(statbuf->st_size + 511) / 512;
	posixTimestamp(&info->ftLastAccessTime, &statbuf->st_atim);
	posixTimestamp(&info->ftLastWriteTime, &statbuf->st_mtim);
	posixTimestamp(&info->ftCreationTime, &statbuf->st_ctim);
}

// TBD figure out what some of these fields mean on Windows.
int xplat_stat(const char *pathname, struct xplat_stat *statbuf)
{
#if 1
	WIN32_FILE_ATTRIBUTE_DATA info;
    BOOL ok = GetFileAttributesExA(pathname, GetFileExInfoStandard, &info);
	if (!ok)
		return -1;

	stat_to_xplat(&info, statbuf);
#else
	// Eventually, I figured out that FindExInfoBasic means "don't set cAlternateFileName",
	// which we never want (that's the 8.3 name, ugg). Setting this flag means a faster
	// iteration.
	WIN32_FIND_DATAA info;
	HANDLE h = FindFirstFileExA(pathname, FindExInfoBasic, &info, FindExSearchNameMatch, NULL, 0);
	if (h == INVALID_HANDLE_VALUE)
		return -1;
	findinfo_to_xplat(&info, statbuf);
#endif
	return 0;
}

int xplat_fstat(int fd, struct xplat_stat *statbuf)
{
    BY_HANDLE_FILE_INFORMATION info;
	BOOL ok = GetFileInformationByHandle(fileIds[fd-3], &info);
	if (!ok)
		return -1;
	fileinfo_to_xplat(&info, statbuf);
	return 0;
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
