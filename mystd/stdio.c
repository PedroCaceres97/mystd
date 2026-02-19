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

typedef struct {
	char* end;
	char* buffer;
	size_t written;
} MyVsnWriter;

typedef enum {
	MY_VSN_LEN_NONE, 
    MY_VSN_LEN_HH, 
    MY_VSN_LEN_H, 
    MY_VSN_LEN_L, 
    MY_VSN_LEN_LL
} MyVsnLengthModifier;

typedef struct {
    char specifier;
	int flag_plus;
	int flag_minus;
	int flag_space;
	int flag_zero;
	int width;
	int precision;
	MyVsnLengthModifier length;
} MyVsnFormatSpec;

static inline void MyVsnWriteChar(MyVsnWriter* writer, char ch);
static inline void MyVsnWriteStr(MyVsnWriter* writer, const char* string, size_t count);
static inline void MyVsnWriteSet(MyVsnWriter* writer, char ch, size_t count);
static inline void MyVsnWritePadded(MyVsnWriter* writer, char pad, size_t pad_count, int left_align, const char* string, size_t count);

static const char* MyVsnParseFlags(const char* format, MyVsnFormatSpec* fs);
static const char* MyVsnParseWidth(const char* format, MyVsnFormatSpec* fs, va_list* args);
static const char* MyVsnParsePrecision(const char* format, MyVsnFormatSpec* fs, va_list* args);
static const char* MyVsnParseLength(const char* format, MyVsnFormatSpec* fs);

static inline int32_t MyVsnGetSigned32(MyVsnFormatSpec* fs, va_list* args);
static inline uint32_t MyVsnGetUnsigned32(MyVsnFormatSpec* fs, va_list* args);
static inline int64_t MyVsnGetSigned64(MyVsnFormatSpec* fs, va_list* args);
static inline uint64_t MyVsnGetUnsigned64(MyVsnFormatSpec* fs, va_list* args);

static void MyVsnFormatChar(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args);
static void MyVsnFormatString(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args);
static void MyVsnFormatSigned(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args);
static void MyVsnFormatUnsigned(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args);
static void MyVsnFormatHexadecimal(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args);
static void MyVsnFormatFloat(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args);
static void MyVsnFormatPointer(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args);
static void MyVsnFormatWritten(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args);

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
	MY_ASSERT_PTR(args);

	MyVsnWriter writer = {.buffer = buffer, .end = buffer ? buffer + max : NULL, .written = 0};
	while (*format) {
		if (*format != '%') {
			MyVsnWriteChar(&writer, *format++);
			continue;
		}

		format++;
		if (*format == '%') {
			MyVsnWriteChar(&writer, *format++);
			continue;
		}

		MyVsnFormatSpec fs = {0};
        fs.width = MY_VSN_UNSET;
        fs.precision = MY_VSN_UNSET;
		format = MyVsnParseFlags(format, &fs);
		format = MyVsnParseWidth(format, &fs, &args);
		format = MyVsnParsePrecision(format, &fs, &args);
		format = MyVsnParseLength(format, &fs);
		fs.specifier = *format++;

		switch(fs.specifier) {
		case 'c':
			MyVsnFormatChar(&writer, &fs, &args);
			break;
		case 's':
			MyVsnFormatString(&writer, &fs, &args);
			break;
		case 'd':
		case 'i':
			MyVsnFormatSigned(&writer, &fs, &args);
			break;
		case 'u':
			MyVsnFormatUnsigned(&writer, &fs, &args);
			break;
		case 'x':
			MyVsnFormatHexadecimal(&writer, &fs, &args);
			break;
		case 'f':
			MyVsnFormatFloat(&writer, &fs, &args);
			break;
		case 'p':
			MyVsnFormatPointer(&writer, &fs, &args);
			break;
		case 'n':
			MyVsnFormatWritten(&writer, &fs, &args);
			break;
		default:
            char temp[256] = {0};
            MyRawSnprintf(temp, sizeof(temp), "Invalid character specifier '%c'", fs.specifier);
            MY_ASSERT(false, temp);
			break;
		}
	}

    if (writer.buffer) { *writer.buffer = 0; } 
	return writer.written;
}

static inline void MyVsnWriteChar(MyVsnWriter* writer, char ch) {
    if (writer->buffer < writer->end) {
        *writer->buffer++ = ch;
	}
    writer->written++;
}
static inline void MyVsnWriteStr(MyVsnWriter* writer, const char* string, size_t count) {
	if (writer->buffer == NULL) {
        writer->written += count;
        return;
    }
    
    size_t available = MY_MIN(count, writer->end - writer->buffer);
    if (available == 0) { return; }

	memcpy(writer->buffer, string, available);
	writer->buffer += available;
    writer->written += count;
}
static inline void MyVsnWriteSet(MyVsnWriter* writer, char ch, size_t count) {
	if (writer->buffer == NULL) {
        writer->written += count;
        return;
    }
    
    size_t available = MY_MIN(count, writer->end - writer->buffer);
    if (available == 0) { return; }

	memset(writer->buffer, ch, available);
	writer->buffer += available;
    writer->written += count;
}
static inline void MyVsnWritePadded(MyVsnWriter* writer, char pad, size_t pad_count, int left_align, const char* string, size_t count) {
	if (left_align) {
		MyVsnWriteStr(writer, string, count);
		MyVsnWriteSet(writer, pad, pad_count);
		return;
	}

	MyVsnWriteSet(writer, pad, pad_count);
	MyVsnWriteStr(writer, string, count);
}

static const char* MyVsnParseFlags(const char* format, MyVsnFormatSpec* fs) {
    while (true) {
        if (*format == '+') { fs->flag_plus = true; format++; }
        else if (*format == '-') { fs->flag_minus = true; format++; }
        else if (*format == ' ') { fs->flag_space = true; format++; }
        else if (*format == '0') { fs->flag_zero = true; format++; }
        else { break; }
    }

    if (fs->flag_plus) { fs->flag_space = false; }
    if (fs->flag_minus) { fs->flag_zero = false; }
    return format;
}
static const char* MyVsnParseWidth(const char* format, MyVsnFormatSpec* fs, va_list* args) {
    if (*format == '*') {
        fs->width = MY_MAX(va_arg(*args, int), 0);
        return format + 1;
    }

    if (!isdigit(*format)) return format;

    fs->width = 0;
    while (isdigit(*format)) {
        fs->width = fs->width * 10 + (*format - '0');
        format++;
    }
    return format;
}
static const char* MyVsnParsePrecision(const char* format, MyVsnFormatSpec* fs, va_list* args) {
    if (*format != '.') { 
		return format; 
	}
	
    format++;
    if (*format == '*') {
        fs->width = MY_MAX(va_arg(*args, int), 0);
        return format + 1;
    }

    if (!isdigit(*format)) return format;

    fs->width = 0;
    while (isdigit(*format)) {
        fs->width = fs->width * 10 + (*format - '0');
        format++;
    }
    return format;
}
static const char* MyVsnParseLength(const char* format, MyVsnFormatSpec* fs) {
    if (*format == 'h') {
        format++;
        fs->length = MY_VSN_LEN_H;
        if (*format == 'h') {
            format++;
            fs->length = MY_VSN_LEN_HH;
        }
        return format;
    }

    if (*format == 'l') {
        format++;
        fs->length = MY_VSN_LEN_L;
        if (*format == 'l') {
            format++;
            fs->length = MY_VSN_LEN_LL;
        }
        return format;
    }

    return format;
}

static inline int32_t MyVsnGetSigned32(MyVsnFormatSpec* fs, va_list* args) {
    MY_ASSERT(fs->length != MY_VSN_LEN_LL, "Trying to fetch a signed 32 bit integer when fs->length is MY_VSN_LEN_LL");
    if (fs->length == MY_VSN_LEN_HH) return (int32_t)va_arg(*args, int);
    if (fs->length == MY_VSN_LEN_H)  return (int32_t)va_arg(*args, int);
    if (fs->length == MY_VSN_LEN_L)  return (int32_t)va_arg(*args, long);
    return va_arg(*args, int);
}
static inline uint32_t MyVsnGetUnsigned32(MyVsnFormatSpec* fs, va_list* args) {
    MY_ASSERT(fs->length != MY_VSN_LEN_LL, "Trying to fetch an unsigned 32 bit integer when fs->length is MY_VSN_LEN_LL");
    if (fs->length == MY_VSN_LEN_HH) return (uint32_t)va_arg(*args, unsigned int);
    if (fs->length == MY_VSN_LEN_H)  return (uint32_t)va_arg(*args, unsigned int);
    if (fs->length == MY_VSN_LEN_L)  return (uint32_t)va_arg(*args, unsigned long);
    return va_arg(*args, unsigned int);
}
static inline int64_t MyVsnGetSigned64(MyVsnFormatSpec* fs, va_list* args) {
    MY_ASSERT(fs->length == MY_VSN_LEN_LL, "Trying to fetch a signed 64 bit integer when fs->length is not MY_VSN_LEN_LL");
    return (int64_t)va_arg(*args, long long);
}
static inline uint64_t MyVsnGetUnsigned64(MyVsnFormatSpec* fs, va_list* args) {
    MY_ASSERT(fs->length == MY_VSN_LEN_LL, "Trying to fetch an unsigned 64 bit integer when fs->length is not MY_VSN_LEN_LL");
    return (uint64_t)va_arg(*args, unsigned long long);
}

static void MyVsnFormatChar(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args) {
    char ch = (char)va_arg(*args, int);

    if (fs->width <= 1) {
        MyVsnWriteChar(writer, ch);
        return;
    }

    MyVsnWritePadded(writer, ' ', fs->width - 1, fs->flag_minus, &ch, 1);
}
static void MyVsnFormatString(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args) {
	const char* value = va_arg(*args, const char*);
	size_t length = strlen(value);

	if (fs->precision != MY_VSN_UNSET && fs->precision < length) {
		length = fs->precision;
	}

	if (fs->width == MY_VSN_UNSET || fs->width < length) {
		MyVsnWriteStr(writer, value, length);
		return;
	}

	MyVsnWritePadded(writer, ' ', fs->width - length, fs->flag_minus, value, length);
}
static void MyVsnFormatSigned(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args) {	
    const char* str = NULL;
    
    if (fs->length == MY_VSN_LEN_LL) {
        int64_t v = MyVsnGetSigned64(fs, args);
        if (v == 0 && fs->precision == 0) return;
        str = MyI64tos(v, fs->flag_plus, fs->flag_space);
    } else {
        int32_t v = MyVsnGetSigned32(fs, args);
        if (v == 0 && fs->precision == 0) return;
        str = MyI32tos(v, fs->flag_plus, fs->flag_space);
    }
    
    size_t length = strlen(str);
	if (fs->width == MY_VSN_UNSET || fs->width < length) {
		MyVsnWriteStr(writer, str, length);
		return;
	}

    char pad = MY_TERNARY(fs->flag_zero, '0', ' ');
    MyVsnWritePadded(writer, pad, fs->width - length, fs->flag_minus, str, length);
}
static void MyVsnFormatUnsigned(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args) {
    const char* str = NULL;
    
    if (fs->length == MY_VSN_LEN_LL) {
        uint64_t v = MyVsnGetUnsigned64(fs, args);
        if (v == 0 && fs->precision == 0) return;
        str = MyU64tos(v, fs->flag_plus, fs->flag_space);
    } else {
        uint32_t v = MyVsnGetUnsigned32(fs, args);
        if (v == 0 && fs->precision == 0) return;
        str = MyU32tos(v, fs->flag_plus, fs->flag_space);
    }
    
    size_t length = strlen(str);
	if (fs->width == MY_VSN_UNSET || fs->width < length) {
		MyVsnWriteStr(writer, str, length);
		return;
	}

    char pad = MY_TERNARY(fs->flag_zero, '0', ' ');
    MyVsnWritePadded(writer, pad, fs->width - length, fs->flag_minus, str, length);
}
static void MyVsnFormatHexadecimal(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args) {
    const char* str = NULL;
    
    if (fs->length == MY_VSN_LEN_LL) {
        uint64_t v = MyVsnGetUnsigned64(fs, args);
        if (v == 0 && fs->precision == 0) return;
        str = MyX64tos(v);
    } else {
        uint32_t v = MyVsnGetUnsigned32(fs, args);
        if (v == 0 && fs->precision == 0) return;
        str = MyX32tos(v);
    }
    
    size_t length = strlen(str);
	if (fs->width == MY_VSN_UNSET || fs->width < length) {
		MyVsnWriteStr(writer, str, length);
		return;
	}

    MyVsnWritePadded(writer, ' ', fs->width - length, fs->flag_minus, str, length);
}
static void MyVsnFormatFloat(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args) {
    const char* str = NULL;
    
    double v = va_arg(*args, double);
    str = MyF64tos(v, MY_TERNARY(fs->precision == MY_VSN_UNSET, 6, fs->precision), fs->flag_plus, fs->flag_space);
    
    size_t length = strlen(str);
	if (fs->width == MY_VSN_UNSET || fs->width < length) {
		MyVsnWriteStr(writer, str, length);
		return;
	}

    MyVsnWritePadded(writer, ' ', fs->width - length, fs->flag_minus, str, length);
}
static void MyVsnFormatPointer(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args) {
    void* value = va_arg(*args, void*);
    const char* str = MyPtrtos(value);

    size_t length = strlen(str);
	if (fs->width == MY_VSN_UNSET || fs->width < length) {
		MyVsnWriteStr(writer, str, length);
		return;
	}

    MyVsnWritePadded(writer, ' ', fs->width - length, fs->flag_minus, str, length);
}
static void MyVsnFormatWritten(MyVsnWriter* writer, MyVsnFormatSpec* fs, va_list* args) {
    size_t* ptr = va_arg(*args, size_t*);
	*ptr = writer->written;
}