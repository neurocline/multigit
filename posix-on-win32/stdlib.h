// stdlib.h

#ifndef _POSIX_ON_WIN32_STDLIB_
#define _POSIX_ON_WIN32_STDLIB_
#pragma message("posix-on-win32/search.h")

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
// Stdlib macros from C Standard

// predefined values for exit(int status)
#define	EXIT_FAILURE 1
#define	EXIT_SUCCESS 0

// Max length of multibyte character. We are lazy, assume UTF-8,
// and no one uses multibyte code from the standard library, do they?
#define	MB_CUR_MAX	4

// If no one else has defined NULL, let's define it
#ifndef NULL
#ifndef __cplusplus
#define NULL ((void *)0)
#else
#define NULL 0
#endif
#endif

// largest return value from rand()
#define	RAND_MAX	0x7fff // qq, Microsft rand is so small

// -----------------------------------------------------------------------------------------------
// Stdlib types from C Standard

_PW_BEGIN_NAMESPACE_STD

// result of div()
typedef struct
  {
    int quot;
    int rem;
  } div_t;

// result of ldiv()
typedef struct
  {
    long quot;
    long rem;
  } ldiv_t;

// result of lldiv()
typedef struct
  {
    long long quot;
    long long rem;
  } lldiv_t;

_PW_END_NAMESPACE_STD

// -----------------------------------------------------------------------------------------------
// Stdlib functions from C Standard

#ifdef	__cplusplus
extern "C" {
#endif

void __cdecl        abort(void);
int __cdecl         abs(int n);
int __cdecl         atexit(void (*func)(void));
double __cdecl      atof(const char* str);
int __cdecl         atoi(const char * str);
long __cdecl        atol( const char * str );
long long __cdecl   atoll( const char * str );
void* __cdecl       bsearch(const void* key, const void* base,
                            size_t num, size_t size,
                            int (*compar)(const void*,const void*));
void* __cdecl       calloc(size_t num, size_t size);
div_t __cdecl       div(int numer, int denom);
void __cdecl        exit(int status);
void __cdecl        free(void* ptr);
char* __cdecl       getenv(const char* name);
long __cdecl        labs(long n);
ldiv_t __cdecl      ldiv(long numer, long denom);
long long __cdecl   llabs (long long n);
lldiv_t __cdecl     lldiv (long long numer, long long denom);
void* __cdecl       malloc(size_t size);
int __cdecl         mblen (const char* pmb, size_t max);
size_t __cdecl      mbstowcs (wchar_t* dest, const char* src, size_t max);
int __cdecl         mbtowc(wchar_t* pwc, const char* pmb, size_t max);
void __cdecl        qsort(void* base, size_t num, size_t size,
                          int (*compar)(const void*,const void*));
int __cdecl         rand(void);
void* __cdecl       realloc(void* ptr, size_t size);
void __cdecl        srand(unsigned int seed);
double __cdecl      strtod(const char* str, char** endptr);
float __cdecl       strtof(const char* str, char** endptr);
long __cdecl        strtol(const char* str, char** endptr, int base);
long double __cdecl strtold(const char* str, char** endptr);
long long __cdecl   strtoll(const char* str, char** endptr, int base);
unsigned long __cdecl strtoul (const char* str, char** endptr, int base);
unsigned long long __cdecl strtoull(const char* str, char** endptr, int base);
int __cdecl         system(const char* command);
size_t __cdecl      wcstombs(char* dest, const wchar_t* src, size_t max);
int __cdecl         wctomb(char* pmb, wchar_t wc);
void __cdecl        _Exit(int status);

// C11
//int __cdecl         at_quick_exit(void (*func)(void));
//_Noreturn void __cdecl quick_exit(int status); // C11

#ifdef	__cplusplus
}
#endif

// -----------------------------------------------------------------------------------------------
// Stdlib macros from Posix

// Define these:
//   WNOHANG
//   WUNTRACED
//   WEXITSTATUS
//   WIFEXITED
//   WIFSIGNALED
//   WIFSTOPPED
//   WSTOPSIG
//   WTERMSIG

// -----------------------------------------------------------------------------------------------
// Stdlib types from Posix

// For size_t and wchar_t
#include <stddef.h>

// -----------------------------------------------------------------------------------------------
// Stdlib functions from Posix

long          a64l(const char *);
double        drand48(void);
//char         *ecvt(double, int, int *restrict, int *restrict);
double        erand48(unsigned short[3]);
//char         *fcvt(double, int, int *restrict, int *restrict);
char         *gcvt(double, int, char *);
int           getsubopt(char **, char *const *, char **);
int           grantpt(int);
char         *initstate(unsigned, char *, size_t);
long          jrand48(unsigned short[3]);
char         *l64a(long);
void          lcong48(unsigned short[7]);
long          lrand48(void);
char* __cdecl mktemp(char *); // legacy
int           mkstemp(char *);
long          mrand48(void);
long          nrand48(unsigned short[3]);
int           posix_memalign(void **, size_t, size_t);
int           posix_openpt(int);
char         *ptsname(int);
int           putenv(char *);
int           rand_r(unsigned *);
long          random(void);
//char         *realpath(const char *restrict, char *restrict);
unsigned short seed48(unsigned short[3]);
int           setenv(const char *, const char *, int);
void          setkey(const char *);
char         *setstate(const char *);
void          srand48(long);
void          srandom(unsigned);
int           unlockpt(int);
int           unsetenv(const char *);

#endif // _POSIX_ON_WIN32_STDLIB_
