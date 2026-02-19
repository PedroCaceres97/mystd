#ifndef __MYSTD_STDIO_H__
#define __MYSTD_STDIO_H__

#include <mystd/stdlib.h>

/* --------------------------------------------------------------------------
 * LOG
 * -------------------------------------------------------------------------- */

#ifndef MY_LOG_STDOUT_FILE
    #define MY_LOG_STDOUT_FILE MyStdout()
#endif
#ifndef MY_LOG_STDERR_FILE
    #define MY_LOG_STDERR_FILE MyStderr()
#endif

#ifdef MY_LOG_COLOURED
    #ifndef MY_LOG_COLOR
        #define MY_LOG_COLOR(msg)      MY_ANSI_COLOR(MY_ANSI_FG_256(212), msg)
    #endif
    #ifndef MY_DEBUG_COLOR
        #define MY_DEBUG_COLOR(msg)    MY_ANSI_COLOR(MY_ANSI_FG_256(87),  msg)
    #endif
    #ifndef MY_SUCCESS_COLOR
        #define MY_SUCCESS_COLOR(msg)  MY_ANSI_COLOR(MY_ANSI_FG_256(46),  msg)
    #endif
    #ifndef MY_WARNING_COLOR
        #define MY_WARNING_COLOR(msg)  MY_ANSI_COLOR(MY_ANSI_FG_256(214), msg)
    #endif
    #ifndef MY_ERROR_COLOR
        #define MY_ERROR_COLOR(msg)    MY_ANSI_COLOR(MY_ANSI_FG_256(196), msg)
    #endif
    #ifndef MY_FATAL_COLOR
        #define MY_FATAL_COLOR(msg)    MY_ANSI_COLOR(MY_ANSI_FG_256(165), msg)
    #endif
#else
    #define MY_LOG_COLOR(msg)      msg
    #define MY_DEBUG_COLOR(msg)    msg
    #define MY_SUCCESS_COLOR(msg)  msg
    #define MY_WARNING_COLOR(msg)  msg
    #define MY_ERROR_COLOR(msg)    msg
    #define MY_FATAL_COLOR(msg)    msg
#endif

#ifndef MY_LOG_TITLE
    #define MY_LOG_TITLE        MY_LOG_COLOR("[LOG]")
#endif /* MY_LOG_TITLE */
#ifndef MY_DEBUG_TITLE
    #define MY_DEBUG_TITLE      MY_DEBUG_COLOR("[DEBUG]")
#endif /* MY_DEBUG_TITLE */
#ifndef MY_SUCCESS_TITLE
    #define MY_SUCCESS_TITLE    MY_SUCCESS_COLOR("[SUCCESS]")
#endif /* MY_SUCCESS_TITLE */
#ifndef MY_WARNING_TITLE
    #define MY_WARNING_TITLE    MY_WARNING_COLOR("[WARNING]")
#endif /* MY_WARNING_TITLE */
#ifndef MY_ERROR_TITLE
    #define MY_ERROR_TITLE      MY_ERROR_COLOR("[ERROR]")
#endif /* MY_ERROR_TITLE */
#ifndef MY_FATAL_TITLE 
    #define MY_FATAL_TITLE      MY_FATAL_COLOR("[FATAL]")
#endif /* MY_FATAL_TITLE */

#ifndef MY_LOG_BUFFERS
    #define MY_LOG_BUFFERS 4
#endif /* MY_LOG_BUFFERS */
#ifndef MY_LOG_BUFFER_SIZE
    #define MY_LOG_BUFFER_SIZE 2048
#endif /* MY_LOG_BUFFER_SIZE */

typedef enum {
    MY_LOG,
    MY_DEBUG,
    MY_SUCCESS,
    MY_WARNING,
    MY_ERROR,
    MY_FATAL
} MyLogLevel;

void MyLog_(MyLogLevel level, MyContext context, const char* msg);

#define MyLog(level, msg) MyLog_(level, MY_CONTEXT(NULL), msg);

/* --------------------------------------------------------------------------
 * FILE
 * -------------------------------------------------------------------------- */

typedef enum {
    MY_FILE_FLAG_QUERY   = 0,
    MY_FILE_FLAG_READ    = 1 << 1,
    MY_FILE_FLAG_WRITE   = 1 << 2,
    MY_FILE_FLAG_NEW     = 1 << 3
} MyFileFlag;

typedef enum {
    MY_SEEK_FLAG_BEGIN,
    MY_SEEK_FLAG_CURRENT,
    MY_SEEK_FLAG_END
} MySeekFlag;

typedef struct MyFile MyFile;

MyFile* MyStdin();
MyFile* MyStdout();
MyFile* MyStderr();

void MyFileClose(MyFile* file);
MyFile* MyFileOpen(const char* path, MyFileFlag flag);

size_t MyFileRead(MyFile* file, char* data, size_t max);
size_t MyFileWrite(MyFile* file, const char* data, size_t max);
size_t MyFilePrint(MyFile* file, const char* data);

size_t MyFileSize(MyFile* file);
size_t MyFileSeek(MyFile* file, MySeekFlag flag, ptrdiff_t offset);

void MyMakeDir(const char* dir);
bool MyDirExists(const char* dir);
bool MyFileExists(const char* file);

/* --------------------------------------------------------------------------
 * PRINTF
 * -------------------------------------------------------------------------- */

#ifndef MY_PRINTF_BUFFERS
    #define MY_PRINTF_BUFFERS 16
#endif /* MY_PRINTF_BUFFERS */
#ifndef MY_PRINTF_BUFFER_SIZE
    #define MY_PRINTF_BUFFER_SIZE 1024
#endif /* MY_PRINTF_BUFFER_SIZE */

size_t      MyPrintf(const char* format, ...);
const char* MySprintf(const char* format, ...);
size_t      MyFprintf(MyFile* file, const char* format, ...);
size_t      MySnprintf(char* buffer, size_t max, const char* format, ...);
size_t      MyVsnprintf(char* buffer, size_t max, const char* format, va_list args);

#endif /* __MYSTD_STDIO_H__ */