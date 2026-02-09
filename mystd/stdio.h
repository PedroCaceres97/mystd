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
MyFile* MyFileOpen(MyFile* file, const char* path, MyFileFlag flag);

size_t MyFileRead(MyFile* file, char* data, size_t max);
size_t MyFileWrite(MyFile* file, const char* data, size_t max);
size_t MyFilePrint(MyFile* file, const char* data);

size_t MyFileSize(MyFile* file);
size_t MyFileSeek(MyFile* file, MySeekFlag flag, ssize_t offset);

void MyMakeDir(const char* dir);
bool MyDirExists(const char* dir);
bool MyFileExists(const char* file);

#ifdef MY_STDIO_IMPLEMENTATION

/* --------------------------------------------------------------------------
 * LOG
 * -------------------------------------------------------------------------- */

static thread_local char myLogBuffers[MY_LOG_BUFFERS][MY_LOG_BUFFER_SIZE];
static thread_local uint32_t myLogIndex = 0;

static inline char* MyLogNextBuffer() {
    myLogIndex++;
    if (myLogIndex == MY_LOG_BUFFERS) { myLogIndex = 0; }
    return myLogBuffers[myLogIndex];
}

void MyLog_(MyLogLevel level, MyContext context, const char* msg) {
#ifndef MY_LOG_DISABLE_ALL
    char* buffer = MyLogNextBuffer();
    char* cursor = buffer;
    char* end = buffer + MY_LOG_BUFFER_SIZE;
    MyFile* file = MY_LOG_STDOUT_FILE;
    const char* title = MY_LOG_TITLE;
    switch(level) {
        case MY_DEBUG: { 
            #ifdef MY_LOG_DISABLE 
                return;
            #endif
            title = MY_DEBUG_TITLE;
            break; 
        }
        case MY_SUCCESS: { 
            #ifdef MY_SUCCESS_DISABLE 
                return;
            #endif
            title = MY_SUCCESS_TITLE;
            break; 
        }
        case MY_WARNING: { 
            #ifdef MY_WARNING_DISABLE 
                return;
            #endif
            title = MY_WARNING_TITLE;
            file = MY_LOG_STDERR_FILE;
            break; 
        }
        case MY_ERROR: { 
            #ifdef MY_ERROR_DISABLE 
                return;
            #endif
            title = MY_ERROR_TITLE;
            file = MY_LOG_STDERR_FILE;
            break; 
        }
        case MY_FATAL: { 
            #ifdef MY_FATAL_DISABLE 
                return;
            #endif
            title = MY_FATAL_TITLE;
            file = MY_LOG_STDERR_FILE;
            break; 
        }
        default: { 
            #ifdef MY_LOG_DISABLE 
                return;
            #endif
            title = MY_LOG_TITLE;
            break; 
        }
    }

    cursor = MyRawStrcpy(cursor, end, title);
    cursor = MyRawStrcpy(cursor, end, "\n Context: ");
    cursor = MyRawStrcpy(cursor, end, context.file);
    cursor = MyRawStrcpy(cursor, end, ":");
    cursor = MyU32tos_(cursor, context.line);
    cursor = MyRawStrcpy(cursor, end, " (");
    cursor = MyRawStrcpy(cursor, end, context.func);
    cursor = MyRawStrcpy(cursor, end, ")\n Message: ");
    cursor = MyRawStrcpy(cursor, end, msg);
    cursor = MyRawStrcpy(cursor, end, "\n\n");
    MyFilePrint(file, buffer);
#endif /* MY_LOG_DISABLE_ALL */
}

/* --------------------------------------------------------------------------
 * FILE
 * -------------------------------------------------------------------------- */

#if defined(MY_OS_WINDOWS)

#include <mystd/stdwin.h>

struct MyFile {
    bool allocated;
    HANDLE handle;
    MyFileFlag flag;
};

static struct MyFile myStdin = { .handle = NULL, .flag = MY_FILE_FLAG_READ };
static struct MyFile myStdout = { .handle = NULL, .flag = MY_FILE_FLAG_WRITE };
static struct MyFile myStderr = { .handle = NULL, .flag = MY_FILE_FLAG_WRITE };

static inline int MyFileIsStd(MyFile* file) {
    return file == &myStdin || file == &myStdout || file == &myStderr;
}
static inline void MyInitStdHandlesOnce(void) {
    static int initialized = false;
    if (initialized) return;

    myStdin.handle  = GetStdHandle(STD_INPUT_HANDLE);
    myStdout.handle = GetStdHandle(STD_OUTPUT_HANDLE);
    myStderr.handle = GetStdHandle(STD_ERROR_HANDLE);

    MY_ASSERT_WINHANDLE(myStdin.handle);
    MY_ASSERT_WINHANDLE(myStdout.handle);
    MY_ASSERT_WINHANDLE(myStderr.handle);

    initialized = true;
}

MyFile* MyStdin(void) {
    MyInitStdHandlesOnce();
    return &myStdin;
}
MyFile* MyStdout(void) {
    MyInitStdHandlesOnce();
    return &myStdout;
}
MyFile* MyStderr(void) {
    MyInitStdHandlesOnce();
    return &myStderr;
}

void MyFileClose(MyFile* file) {
    MY_ASSERT(!MyFileIsStd(file), "Trying to close an std file idiot");
    WINBOOL result = CloseHandle(file->handle);
    MY_ASSERT_WINBOOL(result);

    if (file->allocated) {
        MY_FREE(file);
    }
}
MyFile* MyFileOpen(MyFile* file, const char* path, MyFileFlag flag) {
    if (!file) {
        MY_CALLOC(file, struct MyFile, 1);
        file->allocated = true;
    } else {
        file->allocated = false;
    }

    file->flag = flag;

    DWORD access = 0;
    DWORD creation = OPEN_EXISTING;
    
    if (flag & MY_FILE_FLAG_READ) { access |= GENERIC_READ; }
    if (flag & MY_FILE_FLAG_WRITE) { access |= GENERIC_WRITE; }
    if (flag & MY_FILE_FLAG_NEW) { creation = CREATE_ALWAYS; }

    file->handle = CreateFileA(path, access, 0, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
    MY_ASSERT_WINHANDLE(file->handle);
    return file;
}

size_t MyFileRead(MyFile* file, char* data, size_t max) {
    MY_ASSERT_PTR(file);
    MY_ASSERT_PTR(data);
    MY_ASSERT(file->flag & MY_FILE_FLAG_READ, "File is not readable");
    
    if (max == 0) { return 0; }

    DWORD read = 0;
    WINBOOL result = ReadFile(file->handle, data, max, &read, NULL);
    MY_ASSERT_WINBOOL(result);
    return (size_t)read;
}
size_t MyFileWrite(MyFile* file, const char* data, size_t max) {
    MY_ASSERT_PTR(file);
    MY_ASSERT_PTR(data);
    MY_ASSERT(file->flag & MY_FILE_FLAG_WRITE, "File is not writable");

    if (max == 0) { return 0; }

    DWORD written = 0;
    WINBOOL result = WriteFile(file->handle, data, max, &written, NULL);
    MY_ASSERT_WINBOOL(result);
    return (size_t)written;
}
size_t MyFilePrint(MyFile* file, const char* data) {
    MyFileWrite(file, data, strlen(data));
}

size_t MyFileSize(MyFile* file) {
    MY_ASSERT_PTR(file);
    LARGE_INTEGER LISize = {0};
    WINBOOL result = GetFileSizeEx(file->handle, &LISize);
    MY_ASSERT_WINBOOL(result);
    return (size_t)LISize.QuadPart;
}
size_t MyFileSeek(MyFile* file, MySeekFlag flag, ssize_t offset) {
    MY_ASSERT_PTR(file);
    DWORD method = 0;
    switch (flag) {
        case MY_SEEK_FLAG_BEGIN:   method = FILE_BEGIN;     break;
        case MY_SEEK_FLAG_CURRENT: method = FILE_CURRENT;   break;
        case MY_SEEK_FLAG_END:     method = FILE_END;       break;
        default: MY_ASSERT(false, "Invalid seek flag");
    }

    LARGE_INTEGER LINew = {0};
    LARGE_INTEGER LIOffset = {0};
    LIOffset.QuadPart = offset;
    WINBOOL result = SetFilePointerEx(file->handle, LIOffset, &LINew, flag);
    MY_ASSERT_WINBOOL(result);
    return (size_t)LINew.QuadPart;
}

void MyMakeDir(const char* dir) {
    
}
bool MyDirExists(const char* dir) {
    
}
bool MyFileExists(const char* file) {
    
}

void MyWindowsPrintLastError() {
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 
    MY_ASSERT(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL), "Failed to format windows error");
    MyRawError("\nWindows error ");
    MyRawError(MyU32tos((uint32_t)dw, false, false));
    MyRawError(": ");
    MyRawError((char*)lpMsgBuf);
    MyRawError("\n\n");
    LocalFree(lpMsgBuf);
}

#elif defined(MY_OS_LINUX)

#endif /* defined(MY_OS_WINDOWS) */

#endif  /* MY_STDIO_IMPLEMENTATION */

#endif /* __MYSTD_STDIO_H__ */