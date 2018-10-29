// internal_linux.c
// -- Contains platform-specific routines (maybe this should be
//    called platform_internal.h?)

#include "xplat.h"

#ifdef __linux__

#include <sys/stat.h> // for mkdir
#include <stdlib.h> // for mkstemp
#include <unistd.h> // for access, write

int internal_mkstemp(char* template)
{
    return mkstemp(template);
}

unsigned long internal_write(int fd, void* buf, unsigned long size)
{
    return write(fd, buf, size);
}

int internal_access_R_OK(const char *pathname)
{
    return access(pathname, R_OK);
}

int internal_mkdir(const char* dirpath, int mode)
{
	return mkdir(dirpath, mode);
}

// Return 1 if pathname exists and is a directory, or 0 if not
int internal_is_dir(const char* pathname)
{
    struct stat st;
    if (!stat(pathname, &st) < 0 && S_ISDIR(st.st_mode))
        return 1;
    return 0; // patname not found or exists but is not a directory
}

#endif // __linux__
