#ifndef __MYSTD_STDLIB_H__
#define __MYSTD_STDLIB_H__

/* --------------------------------------------------------------------------
 * STDDEF Section
 * -------------------------------------------------------------------------- */

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

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

#define MY_CONTEXT              ((MyContext){NULL, MY_FILE_PATH, __func__, __LINE__})
#define MY_CONTEXT_ALIAS(alias) ((MyContext){(alias), MY_FILE_PATH, __func__, __LINE__})

#ifndef MY_CONTEXT_COLOR
    #define MY_CONTEXT_COLOR(msg) MY_ANSI_TEXT(MY_ANSI_FG_256(244), msg)
#endif
#ifndef MY_MESSAGE_COLOR
    #define MY_MESSAGE_COLOR(msg) MY_ANSI_TEXT(MY_ANSI_FG_256(207), msg)
#endif
#ifndef MY_CONTEXT_LABEL
    #define MY_CONTEXT_LABEL    MY_CONTEXT_COLOR("Context:")
#endif
#ifndef MY_MESSAGE_LABEL
    #define MY_MESSAGE_LABEL    MY_CONTEXT_COLOR("Message:")
#endif

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
    #endif
#else
    #define MY_RWLOCK_TYPE            void*
    #define MY_RWLOCK_INIT(lock)      ((void)0)
    #define MY_RWLOCK_RDLOCK(lock)    ((void)0)
    #define MY_RWLOCK_WRLOCK(lock)    ((void)0)
    #define MY_RWLOCK_RDUNLOCK(lock)  ((void)0)
    #define MY_RWLOCK_WRUNLOCK(lock)  ((void)0)
    #define MY_RWLOCK_DESTROY(lock)   ((void)0)
#endif

/* Standard structure data and behaviour --------------------------------- */

typedef struct MyStructHeader {
    MY_RWLOCK_TYPE  lock;
    bool           allocated;
} MyStructHeader;

#define MY_STRUCT_CREATE_RULE(v, type)  do { if ((v) == NULL) { MY_CALLOC(v, type, 1); (v)->header.allocated = true; } else { (v)->header.allocated = false; } MY_RWLOCK_INIT((v)->header.lock);    } while(0)
#define MY_STRUCT_DESTROY_RULE(v)       do { MY_RWLOCK_DESTROY((v)->header.lock); if (!(v)->header.allocated) { MY_FREE((v)); } (v) = NULL;                                                         } while(0)
#define MY_RWLOCK_DECLARES(vtype, vname, fnprefix) \
    void MY_CONCAT2(fnprefix, _Rdlock)   (vtype* vname); \
    void MY_CONCAT2(fnprefix, _Wrlock)   (vtype* vname); \
    void MY_CONCAT2(fnprefix, _Rdunlock) (vtype* vname); \
    void MY_CONCAT2(fnprefix, _Wrunlock) (vtype* vname);
#define MY_RWLOCK_DEFINES(vtype, vname, fnprefix) \
    void MY_CONCAT2(fnprefix, _Rdlock)   (vtype* vname) { MY_ASSERT_PTR(vname); MY_RWLOCK_RDLOCK(vname->header.lock);   } \
    void MY_CONCAT2(fnprefix, _Wrlock)   (vtype* vname) { MY_ASSERT_PTR(vname); MY_RWLOCK_WRLOCK(vname->header.lock);   } \
    void MY_CONCAT2(fnprefix, _Rdunlock) (vtype* vname) { MY_ASSERT_PTR(vname); MY_RWLOCK_RDUNLOCK(vname->header.lock); } \
    void MY_CONCAT2(fnprefix, _Wrunlock) (vtype* vname) { MY_ASSERT_PTR(vname); MY_RWLOCK_WRUNLOCK(vname->header.lock); } \

/* Wrappers --------------------------------- */

#define MY_STR_IMPL(x) #x
#define MY_CONCAT2_IMPL(a, b) a##b
#define MY_CONCAT3_IMPL(a, b, c) a##b##c

#define MY_STR(x) MY_STR_IMPL(x)
#define MY_CONCAT2(a, b) MY_CONCAT2_IMPL(a, b)
#define MY_CONCAT3(a, b, c) MY_CONCAT3_IMPL(a, b, c)

#define MY_FREE(ptr)                        do { free((void*)(ptr)); (ptr) = NULL; } while(0)
#define MY_FREE_IF(ptr)                     do { if ((ptr)) { MY_FREE((ptr)); }  } while(0)
#define MY_MALLOC(v, type, size)            do { (v) = (type*)malloc((size));                MY_ASSERT_MALLOC((v), type, (size)); memset(v, 0, (size));                         } while(0)
#define MY_CALLOC(v, type, count)           do { (v) = (type*)calloc((count), sizeof(type)); MY_ASSERT_CALLOC((v), type, (count));                                              } while(0)
#define MY_REALLOC(v, type, ptr, size)      do { (v) = (type*)realloc((void*)(ptr), (size)); MY_ASSERT_REALLOC((v), type, (size));                                              } while(0)

#define MY_CAST(type, tocast)   ((type)(tocast))
#define MY_TERNARY(cnd, x, y)   ((cnd) ? (x) : (y))

#define MY_MIN(x, y)            (MY_TERNARY(x > y, y, x))
#define MY_MAX(x, y)            (MY_TERNARY(x > y, x, y))
#define MY_MIN3(x, y, z)        (MY_MIN(x, MY_MIN(y, z)))
#define MY_MAX3(x, y, z)        (MY_MAX(x, MY_MAX(y, z)))
#define MY_MID(x, y, z)         (x + y + z - MY_MAX3(x, y, z) - MY_MIN3(x, y, z))
#define MY_CLAMP(x, min, max)   (MY_TERNARY(x < min, min, MY_TERNARY(x > max, max, x)))
#define MY_DISTANCE(x, y)       (MY_TERNARY(x > y, x - y, y - x))

#define MY_PTR_ADD(ptr, value)  ((void*)((uint8_t*)(ptr) + (value)))
#define MY_PTR_SUB(ptr, value)  ((void*)((uint8_t*)(ptr) - (value)))

/* --------------------------------------------------------------------------
 * STDLIB And STDIO Section
 * -------------------------------------------------------------------------- */

/* String related functions --------------------------------- */

#ifndef MY_TOS_BUFFER_SIZE
    #define MY_TOS_BUFFER_SIZE 128
#endif
#ifndef MY_TOS_BUFFER_COUNT
    #define MY_TOS_BUFFER_COUNT 16
#endif

char* MyU32tos_(uint32_t value, char* out);
char* MyU64tos_(uint64_t value, char* out);

const char* MyU32tos(uint32_t value, bool plus, bool space);
const char* MyU64tos(uint64_t value, bool plus, bool space);

const char* MyI32tos(int32_t value, bool plus, bool space);
const char* MyI64tos(int64_t value, bool plus, bool space);

const char* MyX32tos(uint32_t value);
const char* MyX64tos(uint64_t value);

const char* MyF32tos(float value, int precision, bool plus, bool space);
const char* MyF64tos(double value, int precision, bool plus, bool space);

const char* MyPtrtos(void* value);
const char* MySizetos(size_t value);
const char* MyPtrdifftos(ptrdiff_t value);

#ifndef MY_PATH_BUFFER_SIZE
    #define MY_PATH_BUFFER_SIZE 1024
#endif
#ifndef MY_PATH_BUFFER_COUNT
    #define MY_PATH_BUFFER_COUNT 8
#endif

char* MyNormalizedPath(char* path);
char* MyFirstPathDivisor(char* path);
char* MyLastPathDivisor(char* path);

/* Filesystem --------------------------------- */

typedef enum {
    MY_FILE_QUERY   = 0,
    MY_FILE_READ    = 1 << 1,
    MY_FILE_WRITE   = 1 << 2,
    MY_FILE_NEW     = 1 << 3
} MyFileFlag;

typedef enum {
    MY_SEEK_BEGIN,
    MY_SEEK_CURRENT,
    MY_SEEK_END
} MySeekFlag;

typedef struct MyFile MyFile;

MyFile* MyStdin();
MyFile* MyStdout();
MyFile* MyStderr();

void MyFileClose(MyFile* file);
MyFile* MyFileOpen(const char* path, MyFileFlag flag);
// Allocated buffer must be freed with MY_FREE
uint8_t* MyFileDump(const char* path, size_t* size);

size_t MyFileRead(MyFile* file, char* data, size_t max);
size_t MyFileWrite(MyFile* file, const char* data, size_t max);
size_t MyFilePrint(MyFile* file, const char* data);

size_t MyFileSize(MyFile* file);
size_t MyFileSeek(MyFile* file, MySeekFlag flag, ptrdiff_t offset);

void MyMakeDir(const char* dir);
bool MyDirExists(const char* dir);
bool MyFileExists(const char* file);

/* Printf implementation --------------------------------- */

#ifndef MY_PRINTF_BUFFER_SIZE
    #define MY_PRINTF_BUFFER_SIZE 1024
#endif
#ifndef MY_PRINTF_BUFFER_COUNT
    #define MY_PRINTF_BUFFER_COUNT 16
#endif

size_t MyPrintf(const char* format, ...);
size_t MyFprintf(MyFile* file, const char* format, ...);
const char* MySprintf(const char* format, ...);
size_t MySnprintf(char* buffer, size_t max, const char* format, ...);
size_t MyVsnprintf(char* buffer, size_t max, const char* format, va_list args);

/* ANSI escape code sequences -------------------------- */

#ifndef MY_ANSI_BUFFER_SIZE
    #define MY_ANSI_BUFFER_SIZE 128
#endif
#ifndef MY_ANSI_BUFFER_COUNT
    #define MY_ANSI_BUFFER_COUNT 16
#endif

#define MY_ANSI_ESC  "\x1b["

#define MY_ANSI_TEXT(ansi, text)  ansi text MY_ANSI_RESET

#define MY_ANSI_FG_BLACK            MY_ANSI_ESC "30m"
#define MY_ANSI_FG_RED              MY_ANSI_ESC "31m"
#define MY_ANSI_FG_GREEN            MY_ANSI_ESC "32m"
#define MY_ANSI_FG_YELLOW           MY_ANSI_ESC "33m"
#define MY_ANSI_FG_BLUE             MY_ANSI_ESC "34m"
#define MY_ANSI_FG_MAGENTA          MY_ANSI_ESC "35m"
#define MY_ANSI_FG_CYAN             MY_ANSI_ESC "36m"
#define MY_ANSI_FG_WHITE            MY_ANSI_ESC "37m"

#define MY_ANSI_BG_BLACK            MY_ANSI_ESC "40m"
#define MY_ANSI_BG_RED              MY_ANSI_ESC "41m"
#define MY_ANSI_BG_GREEN            MY_ANSI_ESC "42m"
#define MY_ANSI_BG_YELLOW           MY_ANSI_ESC "43m"
#define MY_ANSI_BG_BLUE             MY_ANSI_ESC "44m"
#define MY_ANSI_BG_MAGENTA          MY_ANSI_ESC "45m"
#define MY_ANSI_BG_CYAN             MY_ANSI_ESC "46m"
#define MY_ANSI_BG_WHITE            MY_ANSI_ESC "47m"

#define MY_ANSI_RESET               MY_ANSI_ESC "0m"
#define MY_ANSI_BOLD                MY_ANSI_ESC "1m"
#define MY_ANSI_DIM                 MY_ANSI_ESC "2m"
#define MY_ANSI_ITALIC              MY_ANSI_ESC "3m"
#define MY_ANSI_UNDERLINE           MY_ANSI_ESC "4m"
#define MY_ANSI_BLINK               MY_ANSI_ESC "5m"
#define MY_ANSI_REVERSE             MY_ANSI_ESC "7m"
#define MY_ANSI_HIDDEN              MY_ANSI_ESC "8m"
#define MY_ANSI_STRIKETHROUGH       MY_ANSI_ESC "9m"
#define MY_ANSI_DOUBLE_UNDER        MY_ANSI_ESC "21m"
#define MY_ANSI_OVERLINE            MY_ANSI_ESC "53m"
#define MY_ANSI_RESET_TERMINAL      MY_ANSI_ESC "c"

#define MY_ANSI_FG_256(n)           MY_ANSI_ESC "38;5;" #n "m"
#define MY_ANSI_BG_256(n)           MY_ANSI_ESC "48;5;" #n "m"
#define MY_ANSI_FG_RGB(r, g, b)     MY_ANSI_ESC "38;2;" #r ";" #g ";" #b "m"
#define MY_ANSI_BG_RGB(r, g, b)     MY_ANSI_ESC "48;2;" #r ";" #g ";" #b "m"

#define MY_ANSI_CURSOR_UP(n)        MY_ANSI_ESC #n "A"
#define MY_ANSI_CURSOR_DOWN(n)      MY_ANSI_ESC #n "B"
#define MY_ANSI_CURSOR_FORWARD(n)   MY_ANSI_ESC #n "C"
#define MY_ANSI_CURSOR_BACK(n)      MY_ANSI_ESC #n "D"
#define MY_ANSI_CURSOR_POS(x, y)    MY_ANSI_ESC #y ";" #x "H"
#define MY_ANSI_CURSOR_HOME         MY_ANSI_ESC "H"

#define MY_ANSI_CURSOR_SAVE         MY_ANSI_ESC "s"
#define MY_ANSI_CURSOR_RESTORE      MY_ANSI_ESC "u"
#define MY_ANSI_CURSOR_HIDE         MY_ANSI_ESC "?25l"
#define MY_ANSI_CURSOR_SHOW         MY_ANSI_ESC "?25h"

#define MY_ANSI_CLEAR_SCREEN        MY_ANSI_ESC "2J"
#define MY_ANSI_CLEAR_LINE          MY_ANSI_ESC "2K"
#define MY_ANSI_CLEAR_TO_END        MY_ANSI_ESC "0J"
#define MY_ANSI_CLEAR_TO_START      MY_ANSI_ESC "1J"
#define MY_ANSI_CLEAR_LINE_END      MY_ANSI_ESC "0K"
#define MY_ANSI_CLEAR_LINE_START    MY_ANSI_ESC "1K"

char* MyAnsiFg256        (uint8_t n);
char* MyAnsiBg256        (uint8_t n);
char* MyAnsiFgRGB        (uint8_t r, uint8_t g, uint8_t b);
char* MyAnsiBgRGB        (uint8_t r, uint8_t g, uint8_t b);

char* MyAnsiCursorUp     (uint16_t n);
char* MyAnsiCursorDown   (uint16_t n);
char* MyAnsiCursorForward(uint16_t n);
char* MyAnsiCursorBack   (uint16_t n);
char* MyAnsiCursorPos    (uint16_t x, uint16_t y);

/* Assertions -------------------------------- */

#ifndef MY_ASSERT_COLOR
    #define MY_ASSERT_COLOR(msg)    MY_ANSI_TEXT(MY_ANSI_FG_256(165), msg)
#endif
#ifndef MY_ASSERT_TITLE
    #define MY_ASSERT_TITLE MY_ASSERT_COLOR("[MYSTD Assert Report]") 
#endif

MY_NORETURN void MyExit();

void MyAssertLog(const char* msg, MyContext context);
void MyAssertBoundsLog(size_t idx, size_t bounds, MyContext context);

#define MY_ASSERT(cnd, format, ...)              do { if (!(cnd))            { MyAssertLog(MySprintf(format, ##__VA_ARGS__), MY_CONTEXT);   MyExit(); } } while(0)
#define MY_ASSERT_PTR(ptr)                       do { if ((ptr) == NULL)     { MyAssertLog("'"#ptr "' is NULL", MY_CONTEXT);                MyExit(); } } while(0)
#define MY_ASSERT_BOUNDS(idx, bound)             do { if ((idx) >= (bound))  { MyAssertBoundsLog(idx, bound, MY_CONTEXT);                   MyExit(); } } while(0)
#define MY_ASSERT_MALLOC(ptr, type, size)        do { if (ptr == NULL)       { MyAssertLog(MySprintf("Malloc failed for "  #ptr " of type " #type " and size %z",  size), MY_CONTEXT);   MyExit(); } } while(0)
#define MY_ASSERT_CALLOC(ptr, type, count)       do { if (ptr == NULL)       { MyAssertLog(MySprintf("Calloc failed for "  #ptr " of type " #type " and count %zu", count), MY_CONTEXT); MyExit(); } } while(0)
#define MY_ASSERT_REALLOC(ptr, type, size)       do { if (ptr == NULL)       { MyAssertLog(MySprintf("Realloc failed for " #ptr " of type " #type " and size %zu",  size), MY_CONTEXT);  MyExit(); } } while(0)

#if defined(MY_ASSERT_DISABLE) && defined(NDEBUG)
    #undef MY_ASSERT
    #undef MY_ASSERT_PTR
    #undef MY_ASSERT_BOUNDS
    #undef MY_ASSERT_MALLOC
    #undef MY_ASSERT_CALLOC
    #undef MY_ASSERT_REALLOC
    #define MY_ASSERT(cnd, msg)                 do { ((void)0); } while(0)
    #define MY_ASSERT_PTR(ptr)                  do { ((void)0); } while(0)
    #define MY_ASSERT_BOUNDS(idx, bound)        do { ((void)0); } while(0)
    #define MY_ASSERT_MALLOC(ptr, type, size)   do { ((void)0); } while(0)
    #define MY_ASSERT_CALLOC(ptr, type, count)  do { ((void)0); } while(0)
    #define MY_ASSERT_REALLOC(ptr, type, size)  do { ((void)0); } while(0)
#endif

/* LOG System ---------------------------- */

#ifndef MY_LOG_STDOUT_FILE
    #define MY_LOG_STDOUT_FILE MyStdout()
#endif
#ifndef MY_LOG_STDERR_FILE
    #define MY_LOG_STDERR_FILE MyStderr()
#endif

#ifndef MY_INFO_COLOR
    #define MY_INFO_COLOR(msg)      MY_ANSI_TEXT(MY_ANSI_FG_256(212), msg)
#endif
#ifndef MY_DEBUG_COLOR
    #define MY_DEBUG_COLOR(msg)     MY_ANSI_TEXT(MY_ANSI_FG_256(87),  msg)
#endif
#ifndef MY_SUCCESS_COLOR
    #define MY_SUCCESS_COLOR(msg)   MY_ANSI_TEXT(MY_ANSI_FG_256(46),  msg)
#endif
#ifndef MY_WARNING_COLOR
    #define MY_WARNING_COLOR(msg)   MY_ANSI_TEXT(MY_ANSI_FG_256(214), msg)
#endif
#ifndef MY_ERROR_COLOR
    #define MY_ERROR_COLOR(msg)     MY_ANSI_TEXT(MY_ANSI_FG_256(196), msg)
#endif
#ifndef MY_FATAL_COLOR
    #define MY_FATAL_COLOR(msg)     MY_ANSI_TEXT(MY_ANSI_FG_256(165), msg)
#endif

#ifndef MY_INFO_TITLE
    #define MY_INFO_TITLE           MY_INFO_COLOR("[INFO]")
#endif
#ifndef MY_DEBUG_TITLE
    #define MY_DEBUG_TITLE          MY_DEBUG_COLOR("[DEBUG]")
#endif
#ifndef MY_SUCCESS_TITLE
    #define MY_SUCCESS_TITLE        MY_SUCCESS_COLOR("[SUCCESS]")
#endif
#ifndef MY_WARNING_TITLE
    #define MY_WARNING_TITLE        MY_WARNING_COLOR("[WARNING]")
#endif
#ifndef MY_ERROR_TITLE
    #define MY_ERROR_TITLE          MY_ERROR_COLOR("[ERROR]")
#endif
#ifndef MY_FATAL_TITLE 
    #define MY_FATAL_TITLE          MY_FATAL_COLOR("[FATAL]")
#endif

typedef enum {
    MY_INFO,
    MY_DEBUG,
    MY_SUCCESS,
    MY_WARNING,
    MY_ERROR,
    MY_FATAL
} MyLogLevel;

void MyLog_(MyLogLevel level, MyContext context, const char* msg);
#define MyLog(level, format, ...) MyLog_(level, MY_CONTEXT, MySprintf(format, ##__VA_ARGS__))

/* Argv parser ---------------------------- */

typedef struct MyArgvFlag {
    char            value[256]; 
    const char*     longName;
    char            shortName;
    bool            expectValue;
    bool            listener;
} MyArgvFlag;

void MyArgvParse(MyArgvFlag** flags, size_t flagsc, const char** argv, int argc, void (*MyArgvUnkownFlagCallback)(const char*));

#endif /* __MYSTD_STDLIB_H__ */