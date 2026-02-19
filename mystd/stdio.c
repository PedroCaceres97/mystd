#include <mystd/stdio.h>

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
    cursor = MyU32tos_(context.line, cursor);
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
    BOOL result = CloseHandle(file->handle);
    MY_ASSERT_WINBOOL(result);
    MY_FREE(file);
}
MyFile* MyFileOpen(const char* path, MyFileFlag flag) {
    MyFile* file = NULL;
    MY_CALLOC(file, struct MyFile, 1);

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
    BOOL result = ReadFile(file->handle, data, max, &read, NULL);
    MY_ASSERT_WINBOOL(result);
    return (size_t)read;
}
size_t MyFileWrite(MyFile* file, const char* data, size_t max) {
    MY_ASSERT_PTR(file);
    MY_ASSERT_PTR(data);
    MY_ASSERT(file->flag & MY_FILE_FLAG_WRITE, "File is not writable");

    if (max == 0) { return 0; }

    DWORD written = 0;
    BOOL result = WriteFile(file->handle, data, max, &written, NULL);
    MY_ASSERT_WINBOOL(result);
    return (size_t)written;
}
size_t MyFilePrint(MyFile* file, const char* data) {
    return MyFileWrite(file, data, strlen(data));
}

size_t MyFileSize(MyFile* file) {
    MY_ASSERT_PTR(file);
    LARGE_INTEGER LISize = {0};
    BOOL result = GetFileSizeEx(file->handle, &LISize);
    MY_ASSERT_WINBOOL(result);
    return (size_t)LISize.QuadPart;
}
size_t MyFileSeek(MyFile* file, MySeekFlag flag, ptrdiff_t offset) {
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
    BOOL result = SetFilePointerEx(file->handle, LIOffset, &LINew, flag);
    MY_ASSERT_WINBOOL(result);
    return (size_t)LINew.QuadPart;
}

void MyMakeDir(const char* path) {
    MY_ASSERT_PTR(path);
    if (MyDirExists(path)) { return; }

    char temp[MY_MAX_PATH] = {0};
    size_t length = strlen(path); 
    MY_ASSERT(length < MY_MAX_PATH, "Path length is bigger than MY_MAX_PATH, assign a bigger value to MY_MAX_PATH");
    strcpy(temp, path);
    MyNormalizePath(temp);

    /* Skip drive letter */
    size_t i = 0;
    if (length >= 2 && temp[1] == ':') { i = 2; }

    while (i < length) {
        if (temp[i] == '\\') {
            temp[i] = '\0';
            MY_ASSERT(_mkdir(temp) == 0 || errno == EEXIST, "_mkdir failed");
            temp[i] = '\\';
        }

        i++;
    }

    MY_ASSERT(_mkdir(temp) == 0 || errno == EEXIST, "_mkdir failed");
}
bool MyDirExists(const char* path) {
    MY_ASSERT_PTR(path);
    MY_ASSERT(strlen(path) < MY_MAX_PATH, "Path length is bigger than MY_MAX_PATH, assign a bigger value to MY_MAX_PATH");
    char temp[MY_MAX_PATH] = {0};
    strncpy(temp, path, MY_MAX_PATH);
    MyNormalizePath(temp);

    struct _stat st;
    return _stat(temp, &st) == 0 && st.st_mode & _S_IFDIR;
}
bool MyFileExists(const char* path) {
    MY_ASSERT_PTR(path);
    MY_ASSERT(strlen(path) < MY_MAX_PATH, "Path length is bigger than MY_MAX_PATH, assign a bigger value to MY_MAX_PATH");
    char temp[MY_MAX_PATH] = {0};
    strncpy(temp, path, MY_MAX_PATH);
    MyNormalizePath(temp);

    struct _stat st;
    return _stat(temp, &st) == 0 && st.st_mode & _S_IFREG;
}

#elif defined(MY_OS_LINUX)

#endif /* defined(MY_OS_WINDOWS) */

/* --------------------------------------------------------------------------
 * PRINTF
 * -------------------------------------------------------------------------- */

#define MY_VSN_UNSET -1

static thread_local char myPrintfBuffers[MY_PRINTF_BUFFERS][MY_PRINTF_BUFFER_SIZE];
static thread_local uint32_t myPrintfIndex = 0;

static inline char* MyPrintfNextBuffer() {
    myPrintfIndex++;
    if (myPrintfIndex == MY_PRINTF_BUFFERS) { myPrintfIndex = 0; }
    return myPrintfBuffers[myPrintfIndex];
}

typedef enum {
	MY_VSN_LEN_NONE, 
    MY_VSN_LEN_HH, 
    MY_VSN_LEN_H, 
    MY_VSN_LEN_L, 
    MY_VSN_LEN_LL,
    MY_VSN_LEN_Z
} MyVsnLengthModifier;

size_t MyPrintf(const char* format, ...) {
    MY_ASSERT_PTR(format);
    va_list args;
    va_start(args, format);
    char buffer[MY_PRINTF_BUFFER_SIZE] = {0};
    size_t written = MyVsnprintf(buffer, MY_PRINTF_BUFFER_SIZE, format, args);
    MyFilePrint(MyStdout(), buffer);
    va_end(args);
    return written;
}
const char* MySprintf(const char* format, ...) {
    MY_ASSERT_PTR(format);
    va_list args;
    va_start(args, format);
    char* buffer = MyPrintfNextBuffer();
    size_t written = MyVsnprintf(buffer, MY_PRINTF_BUFFER_SIZE, format, args);
    va_end(args);
    return buffer;
}
size_t MyFprintf(MyFile* file, const char* format, ...) {
    MY_ASSERT_PTR(format);
    va_list args;
    va_start(args, format);
    char buffer[MY_PRINTF_BUFFER_SIZE] = {0};
    size_t written = MyVsnprintf(buffer, MY_PRINTF_BUFFER_SIZE, format, args);
    MyFilePrint(file, buffer);
    va_end(args);
    return written;
}
size_t MySnprintf(char* buffer, size_t max, const char* format, ...) {
    MY_ASSERT_PTR(format);
    va_list args;
    va_start(args, format);
    size_t written = MyVsnprintf(buffer, max, format, args);
    va_end(args);
    return written;
}
size_t MyVsnprintf(char* buffer, size_t max, const char* format, va_list args) {
	MY_ASSERT_PTR(format);

    char ch = 0;
    size_t count = 0;
    const char* str= NULL;

    size_t written = 0;
	while (*format) {
		if (*format != '%') {
			ch = *format;
			goto write_char;
		}

		format++;

        bool plus = false;
        bool minus = false;
        bool space = false;
        bool zero = false;

        int width = 0;
        bool set_width = false;

        int precision = 0;
        bool set_precision = false;

        MyVsnLengthModifier length = MY_VSN_LEN_NONE;

        /* Flags */

        while (true) {
            if (*format == '+')         { plus = true;  format++; }
            else if (*format == '-')    { minus = true; format++; }
            else if (*format == ' ')    { space = true; format++; }
            else if (*format == '0')    { zero = true;  format++; }
            else { break; }
        }

        if (plus) { space = false; }
        if (minus) { zero = false; }

        /* Width */

        if (*format == '*') {
            format++;
            int v_width = va_arg(args, int);
            if (v_width >= 0) {
                width = v_width;
                set_width = true;
            }
        } else if (isdigit(*format)) {
            set_width = true;
            while (isdigit(*format)) {
                width = width * 10 + (*format - '0');
                format++;
            }
        }

        /* Precision */

        if (*format == '.') {
            format++;
            if (*format == '*') {
                format++;
                int v_precision = va_arg(args, int);
                if (v_precision >= 0) {
                    precision = v_precision;
                    set_precision = true;
                }
            } else if (isdigit(*format)) {
                set_precision = true;
                while (isdigit(*format)) {
                    precision = precision * 10 + (*format - '0');
                    format++;
                }
            } else {
                MY_ASSERT(false, "Founded '.' but no valid precision was parsed");
            }
        }

        /* Length modifier */
		
        if (*format == 'h') {
            format++;
            length = MY_VSN_LEN_H;
            if (*format == 'h') {
                format++;
                length = MY_VSN_LEN_HH;
            }
        } else if (*format == 'l') {
            format++;
            length = MY_VSN_LEN_L;
            if (*format == 'l') {
                format++;
                length = MY_VSN_LEN_LL;
            }
        } else if (*format == 'z') {
            format++;
            length = MY_VSN_LEN_Z;
        }
		
        /* Specifier */

        /* Percentage */
        if (*format == '%') {
            ch = '%';
            goto write_char;
        }

        /* Character */
        if (*format == 'c') {
            ch = (char)va_arg(args, int);
            goto write_char;
        }

        /* String */
        if (*format == 's') {
            str = va_arg(args, const char*);
            if (!str) { str = "(null)"; }
            else {
                size_t len = strlen(str);
                count = len;
                if (set_precision) { count = MY_MIN(len, (size_t)precision); }
            }
            goto write_string;
        }

        /* Signed */
        if (*format == 'i' || *format == 'd') {
            if (length == MY_VSN_LEN_Z) {
                ptrdiff_t n = va_arg(args, ptrdiff_t);
                str = MyPtrdifftos(n);
                count = strlen(str);
                goto write_string;
            }

            if (length == MY_VSN_LEN_LL) {
                long long n = va_arg(args, long long);
                str = MyI64tos((int64_t)n, plus, space);
                count = strlen(str);
                goto write_string;
            }

            int n = va_arg(args, int);
            str = MyI32tos((int32_t)n, plus, space);
            count = strlen(str);
            goto write_string;
        }

        /* Unsigned */
        if (*format == 'u') {
            if (length == MY_VSN_LEN_Z) {
                size_t n = va_arg(args, size_t);
                str = MySizetos(n);
                count = strlen(str);
                goto write_string;
            }

            if (length == MY_VSN_LEN_LL) {
                unsigned long long n = va_arg(args, unsigned long long);
                str = MyU64tos((uint64_t)n, plus, space);
                count = strlen(str);
                goto write_string;
            }

            unsigned int n = va_arg(args, unsigned int);
            str = MyU32tos((uint32_t)n, plus, space);
            count = strlen(str);
            goto write_string;
        }

        /* Hexadecimal */
        if (*format == 'x') {
            if (length == MY_VSN_LEN_LL) {
                uint64_t n = va_arg(args, uint64_t);
                str = MyX64tos(n);
                count = strlen(str);
                goto write_string;
            }

            uint32_t n = va_arg(args, uint32_t);
            str = MyX32tos(n);
            count = strlen(str);
            goto write_string;
        }

        /* float / double */
        if (*format == 'f') {
            double n = va_arg(args, double);
            str = MyF64tos(n, MY_TERNARY(set_precision, precision, 1), plus, space);
            count = strlen(str);
            goto write_string;
        }

        /* Pointer */
        if (*format == 'p') {
            void* ptr = va_arg(args, void*);
            str = MyPtrtos(ptr);
            count = strlen(str);
            goto write_string;
        }

        /* Written */
        /* TODO: Add lenght specifier behaviour */
        if (*format == 'n') {
            size_t* ptr = va_arg(args, size_t*);
	        *ptr = written;
            format++;
            continue;
        }
        
        char temp[256] = {0};
        MyRawSnprintf(temp, sizeof(temp), "Invalid character specifier '%c'", *format);
        MY_ASSERT(false, temp);

    /* Writers */

    write_char:
        str = &ch;
        count = 1;

    /* TODO: Add Zero flag behaviour */
    write_string:

        /* Left Padding ('-' flag absent) */
        if (set_width && count < width && !minus) {
            for (size_t i = 0; i < width - count; i++) { 
                if (buffer && written + 1 < max) { 
                    *buffer++ = ' '; 
                } 
                written++; 
            }
        }

        /* Write Content */
        for (size_t i = 0; i < count && written + 1 < max; i++) { 
            if (buffer && written + 1 < max) { 
                *buffer++ = str[i]; 
            } 
            written++; 
        }

        /* Right Padding ('-' flag present) */
        if (set_width && count < width && minus) {
            for (size_t i = 0; i < width - count; i++) { 
                if (buffer && written + 1 < max) { 
                    *buffer++ = ' '; 
                } 
                written++; 
            }
        }

        format++;
        continue;
	}

    if (buffer) { *buffer = '\0'; } 
	return written;
}