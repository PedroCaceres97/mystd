#include <mystd/stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MY_OS_WINDOWS)
    #include <io.h>
    #include <direct.h>
    #include <windows.h>
    #include <sys/stat.h>

    #define MY_ASSERT_WIN(x) do { if (!(x)) { MyWindowsPrintLastError(MY_CONTEXT); MyExit(); } } while(false)
    #define MY_ASSERT_WINBOOL(x) MY_ASSERT_WIN(x != 0)
    #define MY_ASSERT_WINHANDLE(x) MY_ASSERT_WIN((x) != INVALID_HANDLE_VALUE && (x) != NULL)

    static void MyOutput(const char* msg) {
        DWORD written = 0;
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), msg, strlen(msg), &written, NULL);
    }
    static void MyError(const char* msg) {
        DWORD written = 0;
        WriteFile(GetStdHandle(STD_ERROR_HANDLE), msg, strlen(msg), &written, NULL);
    }

    void MyWindowsPrintLastError(MyContext context) {
        LPVOID lpMsgBuf;
        DWORD dw = GetLastError(); 
        MyAssertLog("Windows API error, next message will provide OS error message", context);
        MY_ASSERT(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL), "Failed to format windows error");
        MyAssertLog(lpMsgBuf, context);
        LocalFree(lpMsgBuf);
    }

    struct MyFile {
        HANDLE handle;
        MyFileFlag flag;
    };

    static struct MyFile myStdin = { .handle = NULL, .flag = MY_FILE_READ };
    static struct MyFile myStdout = { .handle = NULL, .flag = MY_FILE_WRITE };
    static struct MyFile myStderr = { .handle = NULL, .flag = MY_FILE_WRITE };

    static void MyFileEnableAnsi(MyFile* file) {
        DWORD dwMode = 0;
        MY_ASSERT_WINBOOL(GetConsoleMode(file->handle, &dwMode));
        
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
        MY_ASSERT_WINBOOL(SetConsoleMode(file->handle, dwMode));
    }
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

        MyFileEnableAnsi(&myStdout);
        MyFileEnableAnsi(&myStderr);

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

        if (flag & MY_FILE_READ) { access |= GENERIC_READ; }
        if (flag & MY_FILE_WRITE) { access |= GENERIC_WRITE; }
        if (flag & MY_FILE_NEW) { creation = CREATE_ALWAYS; }

        file->handle = CreateFileA(path, access, 0, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL);
        MY_ASSERT_WINHANDLE(file->handle);
        return file;
    }
    // Allocated buffer must be freed with MY_FREE
    uint8_t* MyFileDump(const char* path, size_t* size) {
        MyFile* file = MyFileOpen(path, MY_FILE_READ);
        size_t _size = MyFileSize(file);
        char* bytes = NULL;
        MY_CALLOC(bytes, char, _size);
        MyFileRead(file, bytes, _size);
        MyFileClose(file);
        if (size) { *size = _size; }
        return (uint8_t*)bytes;
    }

    size_t MyFileRead(MyFile* file, char* data, size_t max) {
        MY_ASSERT_PTR(file);
        MY_ASSERT_PTR(data);
        MY_ASSERT(file->flag & MY_FILE_READ, "File is not readable");

        if (max == 0) { return 0; }

        DWORD read = 0;
        BOOL result = ReadFile(file->handle, data, max, &read, NULL);
        MY_ASSERT_WINBOOL(result);
        return (size_t)read;
    }
    size_t MyFileWrite(MyFile* file, const char* data, size_t max) {
        MY_ASSERT_PTR(file);
        MY_ASSERT_PTR(data);
        MY_ASSERT(file->flag & MY_FILE_WRITE, "File is not writable");

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
            case MY_SEEK_BEGIN:   method = FILE_BEGIN;     break;
            case MY_SEEK_CURRENT: method = FILE_CURRENT;   break;
            case MY_SEEK_END:     method = FILE_END;       break;
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

        char* temp = MyNormalizedPath((char*)path);
        size_t len = strlen(temp);

        /* Skip drive letter */
        size_t i = 0;
        if (len >= 2 && temp[1] == ':') { i = 2; }

        while (i < len) {
            if (temp[i] == '\\') {
                temp[i] = '\0';
                MY_ASSERT(_mkdir(temp) == 0 || errno == EEXIST, MySprintf("_mkdir(%s) -> %s", temp, strerror(errno)));
                temp[i] = '\\';
            }

            i++;
        }
        MY_ASSERT(_mkdir(temp) == 0 || errno == EEXIST, "_mkdir failed");
    }
    bool MyDirExists(const char* path) {
        MY_ASSERT_PTR(path);
        char* temp = MyNormalizedPath((char*)path);
        struct _stat st;
        return _stat(temp, &st) == 0 && st.st_mode & _S_IFDIR;
    }
    bool MyFileExists(const char* path) {
        MY_ASSERT_PTR(path);
        char* temp = MyNormalizedPath(temp);
        struct _stat st;
        return _stat(temp, &st) == 0 && st.st_mode & _S_IFREG;
    }
#elif defined(MY_OS_LINUX)

#endif

/* String related functions --------------------------------- */

static const char MY_DIGITS[201] =
    "00010203040506070809"
    "10111213141516171819"
    "20212223242526272829"
    "30313233343536373839"
    "40414243444546474849"
    "50515253545556575859"
    "60616263646566676869"
    "70717273747576777879"
    "80818283848586878889"
    "90919293949596979899";

static const char MY_HEX_PAIR[513] =
	"000102030405060708090A0B0C0D0E0F"
	"101112131415161718191A1B1C1D1E1F"
	"202122232425262728292A2B2C2D2E2F"
	"303132333435363738393A3B3C3D3E3F"
	"404142434445464748494A4B4C4D4E4F"
	"505152535455565758595A5B5C5D5E5F"
	"606162636465666768696A6B6C6D6E6F"
	"707172737475767778797A7B7C7D7E7F"
	"808182838485868788898A8B8C8D8E8F"
	"909192939495969798999A9B9C9D9E9F"
	"A0A1A2A3A4A5A6A7A8A9AAABACADAEAF"
	"B0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF"
	"C0C1C2C3C4C5C6C7C8C9CACBCCCDCECF"
	"D0D1D2D3D4D5D6D7D8D9DADBDCDDDEDF"
	"E0E1E2E3E4E5E6E7E8E9EAEBECEDEEEF"
	"F0F1F2F3F4F5F6F7F8F9FAFBFCFDFEFF";

static const uint64_t MY_POWERS_10[19] = {
    1ULL, 10ULL, 100ULL, 1000ULL, 10000ULL,
    100000ULL, 1000000ULL, 10000000ULL,
    100000000ULL, 1000000000ULL,
    10000000000ULL, 100000000000ULL,
    1000000000000ULL, 10000000000000ULL,
    100000000000000ULL, 1000000000000000ULL,
    10000000000000000ULL, 100000000000000000ULL,
    1000000000000000000ULL
};

static const float MY_ROUNDERS_F32[8] = {
    0.5f,
    0.05f,
    0.005f,
    0.0005f,
    0.00005f,
    0.000005f,
    0.0000005f,
    0.00000005f
};

static const double MY_ROUNDERS_F64[16] = {
    0.5,
    0.05,
    0.005,
    0.0005,
    0.00005,
    0.000005,
    0.0000005,
    0.00000005,
    0.000000005,
    0.0000000005,
    0.00000000005,
    0.000000000005,
    0.0000000000005,
    0.00000000000005,
    0.000000000000005,
    0.0000000000000005
};

static thread_local char myTosBuffers[MY_TOS_BUFFER_COUNT][MY_TOS_BUFFER_SIZE];
static thread_local uint32_t myTosIndex = 0;
static inline char* MyTosNextBuffer() {
    myTosIndex++;
    if (myTosIndex == MY_TOS_BUFFER_COUNT) { myTosIndex = 0; }
    return myTosBuffers[myTosIndex];
}

char* MyU32tos_(uint32_t value, char* out) {
    char tmp[MY_TOS_BUFFER_SIZE];
    char* cursor = tmp + MY_TOS_BUFFER_SIZE;
    *--cursor = '\0';

    while (value >= 100) {
        uint32_t quotient = value / 100;
        uint32_t module = value - quotient * 100;
        cursor -= 2;
        cursor[0] = MY_DIGITS[module * 2 + 0];
        cursor[1] = MY_DIGITS[module * 2 + 1];
        value = quotient;
    }

    if (value < 10) {
        *--cursor = (char)('0' + value);
    } else {
        cursor -= 2;
        cursor[0] = MY_DIGITS[value * 2 + 0];
        cursor[1] = MY_DIGITS[value * 2 + 1];
    }

    while(*cursor) { *out++ = *cursor++; }
    return out;
}
char* MyU64tos_(uint64_t value, char* out) {
    char tmp[MY_TOS_BUFFER_SIZE];
    char* cursor = tmp + MY_TOS_BUFFER_SIZE;
    *--cursor = '\0';

    while (value >= 100) {
        uint64_t quotient = value / 100;
        uint32_t module = value - quotient * 100;
        cursor -= 2;
        cursor[0] = MY_DIGITS[module * 2 + 0];
        cursor[1] = MY_DIGITS[module * 2 + 1];
        value = quotient;
    }

    if (value < 10) {
        *--cursor = (char)('0' + value);
    } else {
        cursor -= 2;
        cursor[0] = MY_DIGITS[value * 2 + 0];
        cursor[1] = MY_DIGITS[value * 2 + 1];
    }

    while(*cursor) { *out++ = *cursor++; }
    return out;
}

const char* MyU32tos(uint32_t value, bool plus, bool space) {
    char* buffer = MyTosNextBuffer();
    char* cursor = buffer;

    if (plus) { *cursor++ = '+'; }
    else if (space) { *cursor++ = ' '; } 

    MyU32tos_(value, cursor);
    return buffer;
}
const char* MyU64tos(uint64_t value, bool plus, bool space) {
    char* buffer = MyTosNextBuffer();
    char* cursor = buffer;

    if (plus) { *cursor++ = '+'; }
    else if (space) { *cursor++ = ' '; } 

    MyU64tos_(value, cursor);
    return buffer;
}

const char* MyI32tos(int32_t value, bool plus, bool space) {
    uint32_t unsigned_v = (uint32_t)value;
    char* buffer = MyTosNextBuffer();
    char* cursor = buffer;
    
    if (value < 0) { *cursor++ = '-'; unsigned_v = (uint32_t)(-(int64_t)value); } 
    else if (plus) { *cursor++ = '+'; }
    else if (space){ *cursor++ = ' '; } 

    MyU32tos_(unsigned_v, cursor);

    return buffer;
}
const char* MyI64tos(int64_t value, bool plus, bool space) {
    uint64_t unsigned_v = (uint64_t)value;
    char* buffer = MyTosNextBuffer();
    char* cursor = buffer;
    
    if (value < 0) { *cursor++ = '-'; unsigned_v = (uint64_t)(-(int64_t)value); } 
    else if (plus) { *cursor++ = '+'; }
    else if (space){ *cursor++ = ' '; } 

    MyU64tos_(unsigned_v, cursor);

    return buffer;
}

const char* MyX32tos(uint32_t value) {
    char* buffer = MyTosNextBuffer();
    char* cursor = buffer;

    *cursor++ = '0';
    *cursor++ = 'x';

    for (int i = 3; i >= 0; --i) {
        uint32_t byte = (value >> (i * 8)) & 0xFF;
        const char* pair = &MY_HEX_PAIR[byte * 2];
        *cursor++ = pair[0];
        *cursor++ = pair[1];
    }

    return buffer;
}
const char* MyX64tos(uint64_t value) {
    char* buffer = MyTosNextBuffer();
    char* cursor = buffer;

    *cursor++ = '0';
    *cursor++ = 'x';

    for (int i = 7; i >= 0; --i) {
        uint32_t byte = (value >> (i * 8)) & 0xFF;
        const char* pair = &MY_HEX_PAIR[byte * 2];
        *cursor++ = pair[0];
        *cursor++ = pair[1];
    }

    return buffer;
}

const char* MyF32tos(float value, int precision, bool plus, bool space) {
    char* buffer = MyTosNextBuffer();
    char* cursor = buffer;

    if (isnan(value)) { return strcpy(buffer, "nan"); }
    if (isinf(value)) { return strcpy(buffer, MY_TERNARY(signbit(value), "-inf", MY_TERNARY(plus, "+inf", "inf"))); }

    if (value < 0.0f) { *cursor++ = '-'; value = -value; }
    else if (plus) { *cursor++ = '+'; }
    else if (space){ *cursor++ = ' '; } 

    precision = MY_CLAMP(precision, 0, 7);
    value += MY_ROUNDERS_F32[precision];

    uint32_t integer = (uint32_t)value;
    cursor = MyU32tos_(integer, cursor);

    if (precision == 0) { goto MyF32tos_end; }

    *cursor++ = '.';
    
    float decimals_f = value - (float)integer;
    while (precision > 0 && (int)(decimals_f * 10.0f) == 0) {
        *cursor++ = '0';
        precision--;
        decimals_f *= 10.0f;
    }

    uint32_t decimals_u = (uint32_t)(decimals_f * MY_POWERS_10[precision]);
    if (decimals_u != 0 && precision > 0) { cursor = MyU32tos_(decimals_u, cursor); }

MyF32tos_end:
    *cursor = '\0';
    return buffer;
}
const char* MyF64tos(double value, int precision, bool plus, bool space) {
    char* buffer = MyTosNextBuffer();
    char* cursor = buffer;

    if (isnan(value)) { return strcpy(buffer, "nan"); }
    if (isinf(value)) { return strcpy(buffer, MY_TERNARY(signbit(value), "-inf", MY_TERNARY(plus, "+inf", "inf"))); }

    if (value < 0.0f) { *cursor++ = '-'; value = -value; }
    else if (plus) { *cursor++ = '+'; }
    else if (space){ *cursor++ = ' '; } 

    precision = MY_CLAMP(precision, 0, 15);
    value += MY_ROUNDERS_F64[precision];

    uint64_t integer = (uint64_t)value;
    cursor = MyU64tos_(integer, cursor);

    if (precision == 0) { goto MyF64tos_end; }

    *cursor++ = '.';

    double decimals_f = value - (double)integer;
    while (precision > 0 && (int)(decimals_f * 10.0f) == 0) {
        *cursor++ = '0';
        precision--;
        decimals_f *= 10.0f;
    }

    uint64_t decimals_u = (uint64_t)(decimals_f * MY_POWERS_10[precision]); 
    if (decimals_u != 0 && precision > 0) { cursor = MyU64tos_(decimals_u, cursor); }

MyF64tos_end:
    *cursor = '\0';
    return buffer;
}

const char* MyPtrtos(void* value) {
#if defined(MY_PTR_32BIT)
    return MyX32tos((uint32_t)value);
#elif defined(MY_PTR_64BIT)
    return MyX64tos((uint64_t)value);
#else
    MY_ASSERT(false, "Unsupported pointer size");
#endif
}
const char* MySizetos(size_t value) {
#if defined(MY_SIZE_32BIT)
    return MyU32tos((uint32_t)value, false, false);
#elif defined(MY_SIZE_64BIT)
    return MyU64tos((uint64_t)value, false, false);
#else
    MY_ASSERT(false, "Unsupported size_t size");
#endif
}
const char* MyPtrdifftos(ptrdiff_t value) {
#if defined(MY_PTRDIFF_32BIT)
    return MyI32tos((uint32_t)value, false, false);
#elif defined(MY_PTRDIFF_64BIT)
    return MyI64tos((uint64_t)value, false, false);
#else
    MY_ASSERT(false, "Unsupported ptrdiff_t size");
#endif
}

static thread_local char myPathBuffers[MY_PATH_BUFFER_COUNT][MY_PATH_BUFFER_SIZE];
static thread_local uint32_t myPathIndex = 0;
static inline char* MyPathNextBuffer() {
    myPathIndex++;
    if (myPathIndex == MY_PATH_BUFFER_COUNT) { myPathIndex = 0; }
    return myPathBuffers[myPathIndex];
}

char* MyNormalizedPath(char* path) {
    MY_ASSERT_PTR(path);
    size_t len = strlen(path);
    char* buffer = MyPathNextBuffer();
    MY_ASSERT(len < MY_PATH_BUFFER_SIZE, "Path lenght (%zu) is bigger than MY_PATH_BUFFER_SIZE (%zu)", len, MY_PATH_BUFFER_SIZE);
    strcpy(buffer, path);
    for (size_t i = 0; i < MY_PATH_BUFFER_SIZE; i++) { 
        #ifdef MY_OS_WINDOWS
            if (buffer[i] == '/') { buffer[i] = '\\'; } 
        #else
            if (buffer[i] == '\\') { buffer[i] = '/'; }
        #endif
    }
    return buffer;
}
char* MyFirstPathDivisor(char* path) {
    char* p = strchr(path, '/');
    if (!p) { p = strchr(path, '\\'); }
    return p;
}
char* MyLastPathDivisor(char* path) {
    char* p = strrchr(path, '/');
    if (!p) { p = strrchr(path, '\\'); }
    return p;
}

/* Printf implementation --------------------------------- */

static thread_local char myPrintfBuffers[MY_PRINTF_BUFFER_COUNT][MY_PRINTF_BUFFER_SIZE];
static thread_local uint32_t myPrintfIndex = 0;
static inline char* MyPrintfNextBuffer() {
    myPrintfIndex++;
    if (myPrintfIndex == MY_PRINTF_BUFFER_COUNT) { myPrintfIndex = 0; }
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
    char* buffer = MyPrintfNextBuffer();
    size_t written = MyVsnprintf(buffer, MY_PRINTF_BUFFER_SIZE, format, args);
    MyFilePrint(MyStdout(), buffer);
    va_end(args);
    return written;
}
size_t MyFprintf(MyFile* file, const char* format, ...) {
    MY_ASSERT_PTR(format);
    va_list args;
    va_start(args, format);
    char* buffer = MyPrintfNextBuffer();
    size_t written = MyVsnprintf(buffer, MY_PRINTF_BUFFER_SIZE, format, args);
    MyFilePrint(file, buffer);
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
        
        // Invalid character
        continue;

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

/* Assertions -------------------------------- */

MY_NORETURN void MyExit() {
  #ifndef ABORT_ON_ERROR
    exit(-1);
  #else
    abort();
  #endif
}

void MyAssertLog(const char* msg, MyContext context) {
    char buffer[2048] = {0};
    MySnprintf(buffer, sizeof(buffer), "[MYSTD ASSERT REPORT]:\n Context: %s:%u (%s)\n Message: %s\n\n", context.file, context.line, context.func, msg);
    MySnprintf(buffer, sizeof(buffer), 
        "\n%s\n %s " 
        MY_ANSI_TEXT(MY_ANSI_ITALIC MY_ANSI_FG_256(189), "%s:%u -> %s()") "\n %s "
        MY_MESSAGE_COLOR("%s") "\n\n", 
        MY_ASSERT_TITLE, MY_CONTEXT_LABEL, context.file, context.line, context.func, MY_MESSAGE_LABEL, msg);
    MyError(buffer);
}
void MyAssertBoundsLog(size_t idx, size_t bounds, MyContext context) {
    char buffer[2048] = {0};
    MySnprintf(buffer, sizeof(buffer), "[MYSTD ASSERT REPORT]:\n Context: %s:%u (%s)\n Message: Index (%zu) out of bounds (%zu)\n\n", context.file, context.line, context.func, idx, bounds);
    MyError(buffer);
}

/* ANSI escape code sequences -------------------------- */

static thread_local char myAnsiBuffers[MY_ANSI_BUFFER_COUNT][MY_ANSI_BUFFER_SIZE];
static thread_local uint32_t myAnsiIndex = 0;
static inline char* MyAnsiNextBuffer() {
    myAnsiIndex++;
    if (myAnsiIndex == MY_ANSI_BUFFER_COUNT) { myAnsiIndex = 0; }
    return myAnsiBuffers[myAnsiIndex];
}

char* MyAnsiFg256(uint8_t n) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';
    *buffer++ = '3';
    *buffer++ = '8';
    *buffer++ = ';';
    *buffer++ = '5';
    *buffer++ = ';';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'm';
    *buffer++ = '\0';
    return buffer;
}
char* MyAnsiBg256(uint8_t n) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';
    *buffer++ = '4';
    *buffer++ = '8';
    *buffer++ = ';';
    *buffer++ = '5';
    *buffer++ = ';';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'm';
    *buffer++ = '\0';
    return buffer;
}
char* MyAnsiFgRGB(uint8_t r, uint8_t g, uint8_t b) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';
    *buffer++ = '3';
    *buffer++ = '8';
    *buffer++ = ';';
    *buffer++ = '2';
    *buffer++ = ';';
    
    buffer = MyU32tos_((uint32_t)r, buffer);
    *buffer++ = ';';

    buffer = MyU32tos_((uint32_t)g, buffer);
    *buffer++ = ';';

    buffer = MyU32tos_((uint32_t)b, buffer);

    *buffer++ = 'm';
    *buffer++ = '\0';
    return buffer;
}
char* MyAnsiBgRGB(uint8_t r, uint8_t g, uint8_t b) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';
    *buffer++ = '4';
    *buffer++ = '8';
    *buffer++ = ';';
    *buffer++ = '2';
    *buffer++ = ';';
    
    buffer = MyU32tos_((uint32_t)r, buffer);
    *buffer++ = ';';

    buffer = MyU32tos_((uint32_t)g, buffer);
    *buffer++ = ';';

    buffer = MyU32tos_((uint32_t)b, buffer);

    *buffer++ = 'm';
    *buffer++ = '\0';
    return buffer;
}

char* MyAnsiCursorUp(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'A';
    *buffer++ = '\0';
    return buffer;
}
char* MyAnsiCursorDown(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'B';
    *buffer++ = '\0';
    return buffer;
}
char* MyAnsiCursorForward(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'C';
    *buffer++ = '\0';
    return buffer;
}
char* MyAnsiCursorBack(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'D';
    *buffer++ = '\0';
    return buffer;
}
char* MyAnsiCursorPos(uint16_t x, uint16_t y) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';
    
    buffer = MyU32tos_((uint32_t)y, buffer);
    *buffer++ = ';';

    buffer = MyU32tos_((uint32_t)x, buffer);

    *buffer++ = 'H';
    *buffer++ = '\0';
    return buffer;
}

/* LOG System ---------------------------- */

void MyLog_(MyLogLevel level, MyContext context, const char* msg) {
#ifndef MY_LOG_DISABLE_ALL
    char buffer[2048] = {0};
    MyFile* file = MY_LOG_STDOUT_FILE;
    const char* title = MY_INFO_TITLE;
    switch(level) {
        case MY_INFO: {
            #ifdef MY_INFO_DISABLE 
                return;
            #endif
            title = MY_INFO_TITLE;
            break; 
        }
        case MY_DEBUG: { 
            #if defined(NDEBUG) || (defined(MY_DEBUG_DISABLE) && !defined(NDEBUG)) 
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
        // Behaves like MY_INFO
        default: {
            #ifdef MY_INFO_DISABLE 
                return;
            #endif
            title = MY_INFO_TITLE;
            break; 
        }
    }

    MySnprintf(buffer, sizeof(buffer), 
        "\n%s\n %s " 
        MY_ANSI_TEXT(MY_ANSI_ITALIC MY_ANSI_FG_256(189), "%s:%u -> %s()") "\n %s "
        MY_MESSAGE_COLOR("%s") "\n\n", 
        title, MY_CONTEXT_LABEL, context.file, context.line, context.func, MY_MESSAGE_LABEL, msg);
    MyFilePrint(file, buffer);
    if (level == MY_FATAL) { MyExit(); }
#endif /* MY_LOG_DISABLE_ALL */
}

/* Argv parser ---------------------------- */

static int MyArgvQsortCmp(const void* a, const void* b) {
    MY_ASSERT_PTR(a);
    MY_ASSERT_PTR(b);
    const MyArgvFlag* f1 = *(const MyArgvFlag* const*)a;
    const MyArgvFlag* f2 = *(const MyArgvFlag* const*)b;

    const char* s1 = f1->longName ? f1->longName : "";
    const char* s2 = f2->longName ? f2->longName : "";

    return strcmp(s1, s2);
}
static int MyArgvBsearchCmp(const void* key, const void* elem) {
    MY_ASSERT_PTR(key);
    MY_ASSERT_PTR(elem);
    const char* str = key;
    const MyArgvFlag* flag = *(const MyArgvFlag* const*)elem;

    const char* name = flag->longName ? flag->longName : "";
    return strcmp(str, name);
}

static bool MyArgvParseShort(MyArgvFlag** shortNameJumpTable, const char* arg, const char* next) { 
    /*
        At this point is it guaranteed that arg starts with '-' and
        has at least another characther following it.
    */
    MyArgvFlag* flag = shortNameJumpTable[(unsigned char)arg[1]];
    if (flag == NULL) { return false; } // NULL indicates that arg[1] characther was not registered

    flag->listener = true;

    if (flag->expectValue) {
        // As arg is guaranteed to have at least two characther we can check for null terminator in index 2 to check wheter value preceeds flag or is in 'next'
        if (arg[2] != '\0') { 
            strncpy(flag->value, &arg[2], 256 - 1); 
            flag->value[256 - 1] = '\0';
            return false;
        } else { 
            MY_ASSERT(next != NULL, "Any short form flag that requires a value must be preceeded by their value -> -[flag][value] o -[flag] [value]"); 
            strncpy(flag->value, next, 256 - 1); 
            flag->value[256 - 1] = '\0';
            return true;
        }
    }
    
    return false;
}
static void MyArgvParseLong(MyArgvFlag** flags, size_t flagsc, const char* arg) {
    /*
        At this point is it guaranteed that arg starts with "--" and
        has at least another characther following it.
    */
    char key[256] = {0};
    size_t i = 0;
    for (const char* p = arg + 2; *p && *p != '=' && i < sizeof(key)-1; p++) {
        key[i++] = *p;
    }
    key[i] = '\0';
    MyArgvFlag** pflag = bsearch(key, flags, flagsc, sizeof(MyArgvFlag*), MyArgvBsearchCmp);
    if (pflag == NULL) { return; }

    MyArgvFlag* flag = *pflag;
    if (flag->longName == NULL) { return; }
    size_t len = strlen(flag->longName);
    flag->listener = true;

    if (flag->expectValue) {
        MY_ASSERT(arg[len + 2] == '=' && arg[len + 3] != '\0', "Any long form flag that requires a value must be preceeded by '=' and their value -> --[flag]=[value]"); 
        strncpy(flag->value, &arg[len + 3], 256 - 1);
        flag->value[256 - 1] = '\0';
    }
}

void MyArgvParse(MyArgvFlag** flags, size_t flagsc, const char** argv, int argc, void (*MyArgvUnkownFlagCallback)(const char*)) {
    if (argc == 0) { return; } 
    MY_ASSERT_PTR(argv);

    qsort(flags, flagsc, sizeof(MyArgvFlag*), MyArgvQsortCmp);
    MyArgvFlag* shortNameJumpTable[256] = {0};
    for (size_t i = 0; i < flagsc; i++) {
        if (flags[i]->shortName == 0) { continue; }
        shortNameJumpTable[(unsigned char)flags[i]->shortName] = flags[i];
    }
    
    for (int i = 0; i < argc; i++) {
        const char* arg = argv[i];
        const char* next = (i + 1 < argc) ? argv[i + 1] : NULL;

        if (arg == NULL) { continue; }
        if (arg[0] != '-') { MyArgvUnkownFlagCallback(arg); continue; } // Ignoring anything that does not begin with '-'
        if (arg[1] == '\0') { MyArgvUnkownFlagCallback(arg); continue; } // Ignoring "-"

        if (arg[1] == '-') {
            if (arg[2] == '\0') { MyArgvUnkownFlagCallback(arg); continue; } // Ignoring "--"
            MyArgvParseLong(flags, flagsc, arg);
            continue;
        }

        if (MyArgvParseShort(shortNameJumpTable, arg, next)) {
            i++;
        }
    }
}

#ifdef __cplusplus
}
#endif