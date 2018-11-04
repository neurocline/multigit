// fcntl.h

#ifndef _POSIX_ON_WIN32_FCNTL_
#define _POSIX_ON_WIN32_FCNTL_

// MSVC style
//#pragma message("posix-on-win32/fcntl.h")

// If including from C++, we want things in the STD namespace
// and defined as extern C. But if we're not including from a C++
// file, then none of that, please.
#if !defined(_PW_BEGIN_NAMESPACE_STD)
#if defined(__cplusplus)
# define _PW_BEGIN_NAMESPACE_STD	namespace std {
# define _PW_END_NAMESPACE_STD	}
# define _PW_USING_NAMESPACE_STD(name) using std::name;
#else
# define _PW_BEGIN_NAMESPACE_STD
# define _PW_END_NAMESPACE_STD
# define _PW_USING_NAMESPACE_STD(name)
#endif
#endif

// -----------------------------------------------------------------------------------------------
// Fcntl Posix macros

// mask for file access modes
#define O_ACCMODE  (O_WRONLY | O_RDWR)

// file access mode flags for open(), openat(), and fcntl()
#define O_RDONLY     0           // Open for reading only
#define O_WRONLY     (1 << 0)    // Open for writing only
#define O_RDWR       (1 << 1)    // Open for reading and writing
#define O_SEARCH     (1 << 2)    // Open directory for search only
#define O_EXEC       (1 << 3)

// file status flags passed to open(), openat(), and fcntl()
#define O_APPEND     (1 << 4)  // Set append mode
#define O_DSYNC      (1 << 5)
#define O_NONBLOCK   (1 << 6)
#define O_RSYNC      (1 << 7)
#define O_SYNC       (1 << 8)

// file creation flags passed to open() and openat()
#define O_CLOEXEC    (1 << 9)
#define O_CREAT      (1 << 10)  // create and open file
#define O_DIRECTORY  (1 << 11)
#define O_EXCL       (1 << 12)  // open only if file doesn't already exist
#define O_NOCTTY     (1 << 13)
#define O_NOFOLLOW   (1 << 14)
#define O_TRUNC      (1 << 15)  // open and truncate
#define O_TTY_INIT   0

// Linux-only (not in POSIX standard)
#define O_TMPFILE    (1 << 16)  // Linux, not Posix

// Windows-only (not in POSIX standard, or in Linux)
// In non-Windows systems, these would just be zeros
#define O_TEXT       (1 << 17)
#define O_BINARY     (1 << 18)
#define O_RAW        O_BINARY

// -----------------------------------------------------------------------------------------------
// Posix types

// -----------------------------------------------------------------------------------------------
// Posix functions

#endif //_POSIX_ON_WIN32_STDLIB_
