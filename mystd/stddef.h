#ifndef __MYSTD_STDDEF_H__
#define __MYSTD_STDDEF_H__

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

/* OS Family -------------------------------------------------------------- */
#if defined(_WIN32) || defined(_WIN64)
    #define MY_OS_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        #define MY_OS_IOS
    #else
        #define MY_OS_MACOS
    #endif
#elif defined(__linux__)
    #define MY_OS_LINUX
#elif defined(__unix__) || defined(__unix)
    #define MY_OS_UNIX
#elif defined(__ANDROID__)
    #define MY_OS_ANDROID
#else
    #define MY_OS_UNKNOWN
#endif

#if defined(MY_OS_LINUX) || defined(MY_OS_MACOS) || defined(MY_OS_UNIX) || defined(MY_OS_ANDROID) || defined(MY_OS_IOS)
    #define MY_OS_POSIX
#endif

/* CPU Architecture ------------------------------------------------------- */
#if defined(__x86_64__) || defined(_M_X64)
  #define MY_CPU_X64
#elif defined(__i386__) || defined(_M_IX86)
  #define MY_CPU_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
  #define MY_CPU_ARM64
#elif defined(__arm__) || defined(_M_ARM)
  #define MY_CPU_ARM32
#else
  #define MY_CPU_UNKNOWN
#endif

/* Pointer Size ----------------------------------------------------------- */
#if UINTPTR_MAX == UINT64_MAX
  #define MY_PTR_64BIT
#elif UINTPTR_MAX == UINT32_MAX
  #define MY_PTR_32BIT
#else 
  #define MY_PTR_UNKOWN
#endif

/* size_t Size ----------------------------------------------------------- */
#if (SIZE_MAX == UINT64_MAX)
    #define MY_SIZE_64BIT
#elif (SIZE_MAX == UINT32_MAX)
    #define MY_SIZE_32BIT
#else
    #define MY_SIZE_UNKOWN
#endif

/* ptrdiff_t Size ----------------------------------------------------------- */
#if (PTRDIFF_MAX == INT64_MAX)
    #define MY_PTRDIFF_64BIT
#elif (PTRDIFF_MAX == INT32_MAX)
    #define MY_PTRDIFF_32BIT
#else
    #define MY_PTRDIFF_UNKOWN
#endif

/* MAX_PATH -------------------------------------------------------------- */
#ifndef MY_MAX_PATH
    #define MY_MAX_PATH 512
#endif /* MY_MAX_PATH */

/* Compiler -------------------------------------------------------------- */
#if defined(__clang__)
  #define MY_COMPILER_CLANG
#elif defined(__GNUC__)
  #define MY_COMPILER_GCC
#elif defined(_MSC_VER)
  #define MY_COMPILER_MSVC
#else
  #define MY_COMPILER_UNKNOWN
#endif

#if defined(MY_COMPILER_GCC) || defined(MY_COMPILER_CLANG)
  #define MY_NORETURN __attribute__((noreturn))
#elif defined(MY_COMPILER_MSVC)
  #define MY_NORETURN __declspec(noreturn)
#else
  #define MY_NORETURN _Noreturn
#endif

#if defined(__FILE_NAME__)
  #define MY_FILE_PATH __FILE_NAME__
#else
  #define MY_FILE_PATH __FILE__
#endif

/* CONTEXT ---------------------------------------------------------------- */

typedef struct MyContext {
  const char* alias;
  const char* file;
  const char* func;
  uint32_t    line;
} MyContext;

#define MY_CONTEXT(alias) ((MyContext){(alias), MY_FILE_PATH, __func__, __LINE__})

/* RWLOCK ----------------------------------------------------------------- */
#if defined(MY_ENABLE_RWLOCK) && !defined(MY_OS_UNKNOWN) 
  #ifdef MY_OS_WINDOWS
    #include <windows.h>

    #define MY_RWLOCK_TYPE            SRWLOCK
    #define MY_RWLOCK_INIT(lock)      InitializeSRWLock(&(lock))
    #define MY_RWLOCK_RDLOCK(lock)    AcquireSRWLockShared(&(lock))
    #define MY_RWLOCK_WRLOCK(lock)    AcquireSRWLockExclusive(&(lock))
    #define MY_RWLOCK_RDUNLOCK(lock)  ReleaseSRWLockShared(&(lock))
    #define MY_RWLOCK_WRUNLOCK(lock)  ReleaseSRWLockExclusive(&(lock))
    #define MY_RWLOCK_DESTROY(lock)   ((void)0) /* SRWLOCK does not require explicit destroy */
  #else
    #include <pthread.h>

    #define MY_RWLOCK_TYPE            pthread_rwlock_t
    #define MY_RWLOCK_INIT(lock)      pthread_rwlock_init(&(lock), NULL)
    #define MY_RWLOCK_RDLOCK(lock)    pthread_rwlock_rdlock(&(lock))
    #define MY_RWLOCK_WRLOCK(lock)    pthread_rwlock_wrlock(&(lock))
    #define MY_RWLOCK_RDUNLOCK(lock)  pthread_rwlock_unlock(&(lock))
    #define MY_RWLOCK_WRUNLOCK(lock)  pthread_rwlock_unlock(&(lock))
    #define MY_RWLOCK_DESTROY(lock)   pthread_rwlock_destroy(&(lock))
  #endif /* MY_OS_WINDOWS */
#else
  #define MY_RWLOCK_TYPE            void*
  #define MY_RWLOCK_INIT(lock)      ((void)0)
  #define MY_RWLOCK_RDLOCK(lock)    ((void)0)
  #define MY_RWLOCK_WRLOCK(lock)    ((void)0)
  #define MY_RWLOCK_RDUNLOCK(lock)  ((void)0)
  #define MY_RWLOCK_WRUNLOCK(lock)  ((void)0)
  #define MY_RWLOCK_DESTROY(lock)   ((void)0)
#endif /* MY_ENABLE_RWLOCK */

#endif /* __MYSTD_STDDEF_H__ */