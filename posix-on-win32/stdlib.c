// stdlib.c

#include "stdlib.h"

#include <string.h>

// Include Windows headers here
#if defined(_WIN32) && !defined(__CYGWIN__) /* Both MinGW and MSVC */
# if !defined(_WIN32_WINNT)
#  define _WIN32_WINNT 0x0600
# endif
#define WIN32_LEAN_AND_MEAN  /* stops windows.h including winsock.h */
#include <winsock2.h>
#include <windows.h>
#endif

// -----------------------------------------------------------------------------------------------

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
	return open(template, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR /*0700*/);
}
