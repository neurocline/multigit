// unistd.h

#ifndef _POSIX_ON_WIN32_UNISTD_
#define _POSIX_ON_WIN32_UNISTD_
#pragma message("posix-on-win32/unistd.h")

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
// Unistd functions from Posix

#ifndef	__ssize_t_defined
typedef int ssize_t;
#define __ssize_t_defined
#endif

// -----------------------------------------------------------------------------------------------
// Unistd functions from Posix

ssize_t write(int fildes, const void *buf, size_t nbyte);

#endif // _POSIX_ON_WIN32_UNISTD_
