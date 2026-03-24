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
        MyLogCtx(MY_ERROR,  context, "Windows API error, next message will provide OS error message");
        MY_ASSERT(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL), "Failed to format windows error");
        MyLogCtx(MY_ERROR,  context, lpMsgBuf);
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
    char* start = buffer;
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
    return start;
}
char* MyAnsiBg256(uint8_t n) {
    char* buffer = MyAnsiNextBuffer();
    char* start = buffer;
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
    return start;
}
char* MyAnsiFgRGB(uint8_t r, uint8_t g, uint8_t b) {
    char* buffer = MyAnsiNextBuffer();
    char* start = buffer;
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
    return start;
}
char* MyAnsiBgRGB(uint8_t r, uint8_t g, uint8_t b) {
    char* buffer = MyAnsiNextBuffer();
    char* start = buffer;
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
    return start;
}

char* MyAnsiCursorUp(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    char* start = buffer;
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'A';
    *buffer++ = '\0';
    return start;
}
char* MyAnsiCursorDown(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    char* start = buffer;
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'B';
    *buffer++ = '\0';
    return start;
}
char* MyAnsiCursorForward(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    char* start = buffer;
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'C';
    *buffer++ = '\0';
    return start;
}
char* MyAnsiCursorBack(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    char* start = buffer;
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'D';
    *buffer++ = '\0';
    return start;
}
char* MyAnsiCursorPos(uint16_t x, uint16_t y) {
    char* buffer = MyAnsiNextBuffer();
    char* start = buffer;
    *buffer++ = '\x1b';
    *buffer++ = '[';
    
    buffer = MyU32tos_((uint32_t)y, buffer);
    *buffer++ = ';';

    buffer = MyU32tos_((uint32_t)x, buffer);

    *buffer++ = 'H';
    *buffer++ = '\0';
    return start;
}

/* Printf implementation --------------------------------- */

/*
MyPrintf syntax:
    %[$ansi][flags][width][.precision][length][specifier]

ansi (optional): 
    '$' followed by some of the next characters that will indicate the ANSI attribute type

    'C' = CLEAR, has to be followed by one of the next numbers
        '0' = SCREEN
        '1' = LINE
        '2' = TO END
        '3' = TO START
        '4' = LINE END
        '5' = LINE START

    'P' = POSITION / CURSOR, has to be followed by one of the next numbers
        '0' = UP, must be followed by a single number (n)
        '1' = DOWN, must be followed by a single number (n)
        '2' = FORWARD, must be followed by a single number (n)
        '3' = BACK, must be followed by a single number (n)
        '4' = POS, must be followed by two numbers (x,y)
        '5' = HOME
        '6' = SAVE
        '7' = RESTORE
        '8' = HIDE
        '9' = SHOW

    'S' = STYLE, has to be followed by one of the next numbers
        '0' = BOLD
        '1' = DIM
        '2' = ITALIC
        '3' = UNDERLINE
        '4' = BLINK
        '5' = REVERSE
        '6' = HIDDEN
        '7' = STRIKETHROUGH
        '8' = DOUBLE_UNDER
        '9' = OVERLINE

    'F' = FOREGROUND, possible values:
        'black'
        'red'
        'green'
        'yellow'
        'blue'
        'magenta'
        'cyan'
        'white'
        one 256 value 
        three 256 values (separeted by ',')

    'B' = BACKGROUND, possible values:
        'black'
        'red'
        'green'
        'yellow'
        'blue'
        'magenta'
        'cyan'
        'white'
        one 256 value 
        three 256 values (separeted by ',')
    

flags (optional):
    '-': the result of the conversion is left-justified within the field (by default it is right-justified).
    '+': the sign of signed conversions is always prepended to the result of the conversion (by default the result is preceded by minus only when it is negative).
    ' ': if the result of a signed conversion does not start with a sign character, or is empty, space is prepended to the result. It is ignored if + flag is present.

width (optional):
    integer value or * that specifies minimum field width. 
    The result is padded with space characters (by default), if required, 
    on the left when right-justified, or on the right if left-justified. 
    In the case when * is used, the width is specified by an additional 
    argument of type int, which appears before the argument to be converted 
    and the argument supplying precision if one is supplied. 

precision (optional):
    '.' followed by integer number or *, or neither that specifies precision of the conversion. 
    In the case when * is used, the precision is specified by an additional 
    argument of type int, which appears before the argument to be converted, 
    but after the argument supplying minimum field width if one is supplied. 
    If the value of this argument is negative, it is ignored. 
    If neither a number nor * is used, the precision is taken as zero.

length (optional):
    'l': int64_t or uint64_t
    'z': ptrdiff_t or size_t

specifier:
    '%': '%'
    'c': char
    's': const char* (if supplied, precision specifies the maximum number of bytes to be written.)
    'i': int32_t; int64_t; ptrdiff_t (signed integers)
    'u': uint32_t; uint64_t; size_t (unsigned integers)
    'x': uint32_t; uint64_t; (unsigned integers in hexadecimal)
    'f': double (precision specifies the exact number of digits to appear after the decimal point character, default precision is 1.)
    'p': void*
    'n': size_t* (Writes into the provided variable the number of characters written so far by this call to the function.)

*/

static thread_local char myPrintfBuffers[MY_PRINTF_BUFFER_COUNT][MY_PRINTF_BUFFER_SIZE];
static thread_local uint32_t myPrintfIndex = 0;
static inline char* MyPrintfGetBuffer() {
    myPrintfIndex++;
    if (myPrintfIndex == MY_PRINTF_BUFFER_COUNT) { myPrintfIndex = 0; }
    return myPrintfBuffers[myPrintfIndex];
}

/* ============================================================
   Buffer
   ============================================================ */

typedef struct {
    char* data;
    size_t max;
    size_t written;
} MyBuffer;

static void MyBuffer_WriteChar(MyBuffer* buffer, char ch, size_t count) {
    if (buffer->written < buffer->max && buffer->data != NULL) {
        memset(&buffer->data[buffer->written], ch, MY_MIN(count, buffer->max - buffer->written));
    }
    buffer->written += count;
}
static void MyBuffer_WriteN(MyBuffer* buffer, const char* src, size_t count) {
    if (buffer->written < buffer->max && buffer->data != NULL) {
        memcpy(&buffer->data[buffer->written], src, MY_MIN(count, buffer->max - buffer->written));
    }
    buffer->written += count;
}
static void MyBuffer_Write(MyBuffer* buffer, const char* src) {
    MyBuffer_WriteN(buffer, src, strlen(src));
}

/* ============================================================
   Format
   ============================================================ */

typedef struct {
    const char* text;
    MyArgs* args;
} MyFormat;

static bool MyFormat_IsOneOf(MyFormat* format, char* of) {
    
}
static bool MyFormat_AdvanceIfEq(MyFormat* format, char ch) {
    if (*format->text == ch) {
        format->text++;
        return true;
    }
    return false;
}
static bool MyFormat_ParseNumber(MyFormat* format, int* data) {
    if (MyFormat_AdvanceIfEq(format, '*')) {
        MyArgsGet(*data, format->args, int32_t, i32);
        return true;
    }

    bool readed = isdigit(*format->text);
    while(isdigit(*format->text)) {
        *data = *data * 10 + (*format->text - '0'); 
        format->text++;
    }
    return readed;
}
static bool MyFormat_ParseNextNumber(MyFormat* format, int* data) {
    if (MyFormat_AdvanceIfEq(format, ',')) {
        MyFormat_AdvanceIfEq(format, ' ');
        MyFormat_ParseNumber(format, data);
        return true;
    }

    return false;
}

/* ============================================================
   Printf Spec
   ============================================================ */

typedef struct {
    bool        plus;
    bool        minus;
    bool        space;

    int         width;
    int         precision;
    bool        setWidth;
    bool        setPrecision;

    bool        lengthL;
    bool        lengthZ;

    char        temp;
    const char* data;
    size_t      length;
} MyPrintfSpec;

static void MyPrintfSpec_ParseFlags(MyPrintfSpec* spec, MyFormat* format) {
    while (true) {
        if (MyFormat_AdvanceIfEq(format, '+')) { spec->plus = true; }
        else if (MyFormat_AdvanceIfEq(format, '-')) { spec->minus = true; }
        else if (MyFormat_AdvanceIfEq(format, ' ')) { spec->space = true; }
        else { break; }
    }
    if (spec->plus) { spec->space = false; }
}
static void MyPrintfSpec_ParseWidth(MyPrintfSpec* spec, MyFormat* format) {
    spec->setWidth = MyFormat_ParseNumber(format, &spec->width);
}
static void MyPrintfSpec_ParsePrecision(MyPrintfSpec* spec, MyFormat* format) {
    if (MyFormat_AdvanceIfEq(format, '.')) {
        spec->setPrecision = true;
        MyFormat_ParseNumber(format, &spec->precision);
    }
}
static void MyPrintfSpec_ParseLenght(MyPrintfSpec* spec, MyFormat* format) {
    if (MyFormat_AdvanceIfEq(format, 'z')) {
        spec->lengthZ = true;
        return;
    }

    if (MyFormat_AdvanceIfEq(format, 'l')) {
        MyFormat_AdvanceIfEq(format, 'l');
        spec->lengthL = true;
        return;
    }

    MyFormat_AdvanceIfEq(format, 'h');
    MyFormat_AdvanceIfEq(format, 'h');
}

static void MyPrintfSpec_ParsePP(MyPrintfSpec* spec, MyFormat* format) {
    spec->temp = '%';
    spec->data = &spec->temp;
    spec->length = 1;
}
static void MyPrintfSpec_ParseC(MyPrintfSpec* spec, MyFormat* format) {
    MyArgsGet(spec->temp, format->args, int32_t, i32);
    spec->data = &spec->temp;
    spec->length = 1;
}
static void MyPrintfSpec_ParseS(MyPrintfSpec* spec, MyFormat* format) {
    MyArgsGet(spec->data, format->args, const char*, str);
    if (!spec->data) { spec->data = "(null)"; }
    spec->length = strlen(spec->data);
    if (spec->setPrecision && spec->length > spec->precision) { spec->length = spec->precision; }
}
static void MyPrintfSpec_ParseI(MyPrintfSpec* spec, MyFormat* format) {
    if (spec->lengthZ) {
        ptrdiff_t value = 0;
        MyArgsGet(value, format->args, ptrdiff_t, dif);
        spec->data = MyPtrdifftos(value);
        spec->length = strlen(spec->data);
        return;
    }
    
    if (spec->lengthL) {
        int64_t value = 0;
        MyArgsGet(value, format->args, int64_t, i64);
        spec->data = MyI64tos(value, spec->plus, spec->space);
        spec->length = strlen(spec->data);
        return;
    } 
    
    int32_t value = 0;
    MyArgsGet(value, format->args, int32_t, i32);
    spec->data = MyI32tos(value, spec->plus, spec->space);
    spec->length = strlen(spec->data);
}
static void MyPrintfSpec_ParseU(MyPrintfSpec* spec, MyFormat* format) {
    if (spec->lengthZ) {
        size_t value = 0;
        MyArgsGet(value, format->args, size_t, sze);
        spec->data = MySizetos(value);
        spec->length = strlen(spec->data);
        return;
    }
    
    if (spec->lengthL) {
        uint64_t value = 0;
        MyArgsGet(value, format->args, uint64_t, u64);
        spec->data = MyU64tos(value, spec->plus, spec->space);
        spec->length = strlen(spec->data);
        return;
    }

    uint32_t value = 0;
    MyArgsGet(value, format->args, uint32_t, i32);
    spec->data = MyU32tos(value, spec->plus, spec->space);
    spec->length = strlen(spec->data);
}
static void MyPrintfSpec_ParseX(MyPrintfSpec* spec, MyFormat* format) {
    if (spec->lengthL) {
        uint64_t value = 0;
        MyArgsGet(value, format->args, uint64_t, i64);
        spec->data = MyX64tos(value);
        spec->length = strlen(spec->data);
        return;
    }
    
    uint32_t value = 0;
    MyArgsGet(value, format->args, uint32_t, i32);
    spec->data = MyX32tos(value);
    spec->length = strlen(spec->data);
}
static void MyPrintfSpec_ParseF(MyPrintfSpec* spec, MyFormat* format) {
    double value = 0;
    MyArgsGet(value, format->args, double, f64);
    spec->data = MyF64tos(value, MY_TERNARY(spec->setPrecision, spec->precision, 1), spec->plus, spec->space);
    spec->length = strlen(spec->data);
}
static void MyPrintfSpec_ParseP(MyPrintfSpec* spec, MyFormat* format) {
    void* value = 0;
    MyArgsGet(value, format->args, void*, ptr);
    spec->data = MyPtrtos(value);
    spec->length = strlen(spec->data);
}
static void (*myPrintfSpecParsers[256])(MyPrintfSpec*, MyFormat*) = {
    [0 ... 255] = NULL,
    ['%'] = MyPrintfSpec_ParsePP,
    ['c'] = MyPrintfSpec_ParseC,
    ['s'] = MyPrintfSpec_ParseS,
    ['i'] = MyPrintfSpec_ParseI,
    ['u'] = MyPrintfSpec_ParseU,
    ['x'] = MyPrintfSpec_ParseX,
    ['f'] = MyPrintfSpec_ParseF,
    ['p'] = MyPrintfSpec_ParseP,
};

void MyPrintfSpec_Parse(MyPrintfSpec* spec, MyFormat* format, MyBuffer* buffer) {
    MyPrintfSpec_ParseFlags(spec, format);
    MyPrintfSpec_ParseWidth(spec, format);
    MyPrintfSpec_ParsePrecision(spec, format);
    MyPrintfSpec_ParseLenght(spec, format);

    if (MyFormat_AdvanceIfEq(format, 'n')) {
        size_t* n = NULL;
        MyArgsGet(n, format->args, size_t*, ptr);
        *n = buffer->written;
        return;
    }

    void (*parser)(MyPrintfSpec*, MyFormat*) = myPrintfSpecParsers[tolower(*format->text++)];
    if (!parser) { return; }

    parser(spec, format);

    if (spec->setWidth && spec->length < spec->width && !spec->minus) { 
        // Left Padding
        MyBuffer_WriteChar(buffer, ' ', spec->width - spec->length); 
    }

    // Write parsed content
    MyBuffer_WriteN(buffer, spec->data, spec->length);

    if (spec->setWidth && spec->length < spec->width && spec->minus) { 
        // Right Padding
        MyBuffer_WriteChar(buffer, ' ', spec->width - spec->length); 
    }
}

/* ============================================================
   ANSI
   ============================================================ */

static const char* myAnsiFgColors[256] = {
    [0 ... 255] = NULL,
    ['k'] = MY_ANSI_FG_BLACK,
    ['r'] = MY_ANSI_FG_RED,
    ['g'] = MY_ANSI_FG_GREEN,
    ['y'] = MY_ANSI_FG_YELLOW,
    ['b'] = MY_ANSI_FG_BLUE,
    ['m'] = MY_ANSI_FG_MAGENTA,
    ['c'] = MY_ANSI_FG_CYAN,
    ['w'] = MY_ANSI_FG_WHITE,
};
static const char* myAnsiBgColors[256] = {
    [0 ... 255] = NULL,
    ['k'] = MY_ANSI_BG_BLACK,
    ['r'] = MY_ANSI_BG_RED,
    ['g'] = MY_ANSI_BG_GREEN,
    ['y'] = MY_ANSI_BG_YELLOW,
    ['b'] = MY_ANSI_BG_BLUE,
    ['m'] = MY_ANSI_BG_MAGENTA,
    ['c'] = MY_ANSI_BG_CYAN,
    ['w'] = MY_ANSI_BG_WHITE,
};
static const char* myAnsiStyles[10] = {
    MY_ANSI_BOLD,
    MY_ANSI_DIM,
    MY_ANSI_ITALIC,
    MY_ANSI_UNDERLINE,
    MY_ANSI_BLINK,
    MY_ANSI_REVERSE,
    MY_ANSI_HIDDEN,
    MY_ANSI_STRIKETHROUGH,
    MY_ANSI_DOUBLE_UNDER,
    MY_ANSI_OVERLINE
};
static const char* myAnsiClears[6] = {
    MY_ANSI_CLEAR_SCREEN,
    MY_ANSI_CLEAR_LINE,
    MY_ANSI_CLEAR_TO_END,
    MY_ANSI_CLEAR_TO_START,
    MY_ANSI_CLEAR_LINE_END,
    MY_ANSI_CLEAR_LINE_START,
};
static const char* myAnsiCursors[10] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    MY_ANSI_CURSOR_HOME,
    MY_ANSI_CURSOR_SAVE,
    MY_ANSI_CURSOR_RESTORE,
    MY_ANSI_CURSOR_HIDE,
    MY_ANSI_CURSOR_SHOW
};
static char* (*myPrintfCursorsX[10])(uint16_t) = {
    MyAnsiCursorUp,
    MyAnsiCursorDown,
    MyAnsiCursorForward,
    MyAnsiCursorBack,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static void MyAnsi_ParseC(MyPrintfSpec* spec, MyFormat* format) {
    int idx = 0;
    MyFormat_ParseNumber(format, &idx);
    spec->data = myAnsiClears[idx % 6];
}
static void MyAnsi_ParseP(MyPrintfSpec* spec, MyFormat* format) {
    int x = 0;
    int y = 0;
    int idx = 0;
    MyFormat_ParseNumber(format, &idx);
    idx = idx % 10;

    if (idx > 4) {
        spec->data = myAnsiCursors[idx];
        return;
    }

    MyFormat_ParseNumber(format, &x);
    if (idx < 4) {
        spec->data = myPrintfCursorsX[idx]((uint16_t)x);
        return;
    }

    MyFormat_ParseNextNumber(format, &y);
    spec->data = MyAnsiCursorPos((uint16_t)x, (uint16_t)y);
}
static void MyAnsi_ParseS(MyPrintfSpec* spec, MyFormat* format) {
    int idx = 0;
    MyFormat_ParseNumber(format, &idx);
    spec->data = myAnsiStyles[idx % 10];
}
static void MyAnsi_ParseF(MyPrintfSpec* spec, MyFormat* format) {
    spec->data = myAnsiFgColors[*format->text];
    if (spec->data) { 
        format->text++;
        return;
    }

    int r = 0;
    int g = 0;
    int b = 0;

    MyFormat_ParseNumber(format, &r);
    if (!MyFormat_ParseNextNumber(format, &g)) {
        spec->data = MyAnsiFg256((uint8_t)r);
        return;
    }

    MyFormat_ParseNextNumber(format, &b);
    spec->data = MyAnsiFgRGB((uint8_t)r, (uint8_t)g, (uint8_t)b);
}
static void MyAnsi_ParseB(MyPrintfSpec* spec, MyFormat* format) {
    spec->data = myAnsiBgColors[*format->text];
    if (spec->data) { 
        format->text++;
        return;
    }

    int r = 0;
    int g = 0;
    int b = 0;

    MyFormat_ParseNumber(format, &r);
    if (!MyFormat_ParseNextNumber(format, &g)) {
        spec->data = MyAnsiBg256((uint8_t)r);
        return;
    }

    MyFormat_ParseNextNumber(format, &b);
    spec->data = MyAnsiBgRGB((uint8_t)r, (uint8_t)g, (uint8_t)b);
}
static void (*myAnsiParsers[256])(MyPrintfSpec*, MyFormat*) = {
    [0 ... 255] = NULL,
    ['C'] = MyAnsi_ParseC,
    ['P'] = MyAnsi_ParseP,
    ['S'] = MyAnsi_ParseS,
    ['F'] = MyAnsi_ParseF,
    ['B'] = MyAnsi_ParseB
};
static bool myAnsiParsersReset[256] = {
    [0 ... 255] = false,
    ['S'] = true,
    ['F'] = true,
    ['B'] = true
};

static bool MyAnsi_Parse(MyPrintfSpec* spec, MyBuffer* buffer, MyFormat* format) {
    if (!format->text) { return false; }
    
    bool reset = false;
    while (*format->text) {
        char key = *format->text++;
        void (*parser)(MyPrintfSpec*, MyFormat*) = myAnsiParsers[key];
        if (parser == NULL) { continue; }
        MyFormat_AdvanceIfEq(format, ':');
        MyFormat_AdvanceIfEq(format, ' ');
        parser(spec, format);
        MyBuffer_Write(buffer, spec->data);
        if (myAnsiParsersReset[key]) { reset = true; }
    }
    return reset;
}

/* ============================================================
   Printf (Standard)
   ============================================================ */

size_t      MyPrintf(const char* format, ...) {
    MY_ASSERT_PTR(format);
    va_list args;
    va_start(args, format);
    char buffer[MY_PRINTF_BUFFER_SIZE];
    size_t written = MyVsnprintf(buffer, MY_PRINTF_BUFFER_SIZE, format, args);
    MyFileWrite(MyStdout(), buffer, written);
    va_end(args);
    return written;
}
size_t      MyFprintf(MyFile* file, const char* format, ...) {
    MY_ASSERT_PTR(format);
    va_list args;
    va_start(args, format);
    char buffer[MY_PRINTF_BUFFER_SIZE];
    size_t written = MyVsnprintf(buffer, MY_PRINTF_BUFFER_SIZE, format, args);
    MyFilePrint(file, buffer);
    va_end(args);
    return written;
}
const char* MySprintf(const char* format, ...) {
    MY_ASSERT_PTR(format);
    va_list args;
    va_start(args, format);
    char* buffer = MyPrintfGetBuffer();
    size_t written = MyVsnprintf(buffer, MY_PRINTF_BUFFER_SIZE, format, args);
    va_end(args);
    return buffer;
}
size_t      MySnprintf(char* buffer, size_t max, const char* format, ...) {
    MY_ASSERT_PTR(format);
    va_list args;
    va_start(args, format);
    size_t written = MyVsnprintf(buffer, max, format, args);
    va_end(args);
    return written;
}
size_t      MyVsnprintf(char* buffer, size_t max, const char* format, va_list args) {
    MyBuffer buf = {0};
    buf.max = max - 1;
    buf.data = buffer;
    buf.written = 0;

    MyArgs arg = {0};
    arg.type = MY_ARGS_STDARG;
    arg.backend.stdarg = args;

    MyFormat fmt = {0};
    fmt.text = format;
    fmt.args = &arg;

    while (true) {
        MyPrintfSpec spec = {0};
        
        const char* percentage = strchr(fmt.text, '%');
        if (percentage == NULL) { 
            MyBuffer_Write(&buf, fmt.text);
            break;
        }

        if (percentage != fmt.text) {
            MyBuffer_WriteN(&buf, fmt.text, MY_PTR_DIF(percentage, fmt.text));
        }

        fmt.text = percentage + 1;
        MyPrintfSpec_Parse(&spec, &fmt, &buf);
    }

    if (buf.data) { buf.data[MY_MIN(buf.written, buf.max)] = '\0'; }
    return buf.written;
}

/* ============================================================
   Printf (Segments)
   ============================================================ */

size_t      MyPrintfSegmentsN(MyPrintfSegment* segments, size_t count) {
    char buffer[MY_PRINTF_BUFFER_SIZE];
    size_t written = MySnprintfSegmentsN(buffer, MY_PRINTF_BUFFER_SIZE, segments, count);
    MyFileWrite(MyStdout(), buffer, written);
    return written;
}
const char* MySprintfSegmentsN(MyPrintfSegment* segments, size_t count) {
    char* buffer = MyPrintfGetBuffer();
    size_t written = MySnprintfSegmentsN(buffer, MY_PRINTF_BUFFER_SIZE, segments, count);
    return buffer;
}
size_t      MySnprintfSegmentsN(char* buffer, size_t max, MyPrintfSegment* segments, size_t count) {
    MyBuffer buf = {0};
    buf.max = max - 1;
    buf.data = buffer;

    for (size_t i = 0; i < count; i++) {
        MyPrintfSpec spec = {0};

        MyArgs arg = {0};
        arg.type = MY_ARGS_MYSTD;
        arg.backend.mystd = segments[i].args;

        MyFormat fmt = {0};
        fmt.args = &arg;
        
        fmt.text = segments[i].ansi;
        bool reset = MyAnsi_Parse(&spec, &buf, &fmt);

        fmt.text = segments[i].format;
        while (true) {
            spec = (MyPrintfSpec){0};

            const char* percentage = strchr(fmt.text, '%');
            if (percentage == NULL) { 
                MyBuffer_Write(&buf, fmt.text);
                break;
            }

            if (percentage != fmt.text) {
                MyBuffer_WriteN(&buf, fmt.text, MY_PTR_DIF(percentage, fmt.text));
            }

            fmt.text = percentage + 1;
            MyPrintfSpec_Parse(&spec, &fmt, &buf);
        }

        if (reset) { MyBuffer_Write(&buf, MY_ANSI_RESET); }
    }

    if (buf.data) { buf.data[MY_MIN(buf.written, buf.max)] = '\0'; }
    return buf.written;
}

/* LOG System ---------------------------- */

MY_NORETURN void MyExit() {
  #ifndef ABORT_ON_ERROR
    exit(-1);
  #else
    abort();
  #endif
}

void MyLogCtx(MyLogLevel level, MyContext context, const char* msg) {
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

    MySnprintfSegments(buffer, sizeof(buffer),
        SEG("", "%s\n", STR(title)),
        SEG("F: 244", "Context: "),
        SEG("F: 189 S: 2", "%s:%u -> %s()\n", STR(context.file), I32(context.line), STR(context.func)),
        SEG("F: 244", "Message: "),
        SEG("F: 207", "%s\n\n", STR(msg))
    );

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

static bool MyArgvParseShort(MyArgvFlag** shortNameJumpTable, const char* arg, const char* next, int* i) { 
    /*
        At this point is it guaranteed that arg starts with '-' and
        has at least another characther following it.
    */
    MyArgvFlag* flag = shortNameJumpTable[(unsigned char)arg[1]];
    if (flag == NULL) { return false; } // NULL indicates that arg[1] characther was not registered

    flag->trigged = true;

    if (flag->expectValue) {
        // As arg is guaranteed to have at least two characther we can check for null terminator in index 2 to check wheter value preceeds flag or is in 'next'
        if (arg[2] != '\0') { 
            strncpy(flag->value, &arg[2], 256 - 1); 
            flag->value[256 - 1] = '\0';
        } else { 
            MY_ASSERT(next != NULL, "Correct Usage: -[short flag][value] or -[short flag] [value]"); 
            strncpy(flag->value, next, 256 - 1); 
            flag->value[256 - 1] = '\0';
            *i = *i + 1;
        }
    }
    
    return true;
}
static bool MyArgvParseLong(MyArgvFlag** flags, size_t flagsc, const char* arg) {
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
    if (pflag == NULL) { return false; }

    MyArgvFlag* flag = *pflag;
    if (flag->longName == NULL) { return false; }
    size_t len = strlen(flag->longName);
    flag->trigged = true;

    if (flag->expectValue) {
        MY_ASSERT(arg[len + 2] == '=' && arg[len + 3] != '\0', "Correct Usage: --[flag]=[value]"); 
        strncpy(flag->value, &arg[len + 3], 256 - 1);
        flag->value[256 - 1] = '\0';
    }
    return true;
}
static bool MyArgvParseHelp(MyArgvFlag** shortNameJumpTable, MyArgvFlag** flags, size_t flagsc, const char* arg) {
    if (arg[1] == '\0') {
        MyArgvFlag* flag = shortNameJumpTable[(unsigned char)arg[1]];
        if (flag == NULL) { return false; } // NULL indicates that arg[1] characther was not registered
        goto print;
    }

    MyArgvFlag** pflag = bsearch(arg, flags, flagsc, sizeof(MyArgvFlag*), MyArgvBsearchCmp);
    if (pflag == NULL) { return false; }

    MyArgvFlag* flag = *pflag;
    if (flag->longName == NULL) { return false; }

print:
    char shortName = MY_TERNARY(flag->shortName == 0, '~', flag->shortName);
    const char* shortDesc = MY_TERNARY(flag->shortName == 0, "(Doesnt have a shortcut)", "");
    const char* expects = MY_TERNARY(flag->expectValue, "Yes", "No");

    MyPrintfSegments(
        SEG("F: 212",       "Flag: "),
        SEG("F: 189 S: 2",  "%s\n",         STR(flag->longName)),
        SEG("F: 212",       "Short: "),
        SEG("F: 189 S: 2",  "%c %s\n",      I32(shortName), STR(shortDesc)),
        SEG("F: 212",       "Expects Value: "),
        SEG("F: 189 S: 2",  "%s\n",         STR(expects)),
        SEG("F: 212",       "Description: "),
        SEG("F: 207",       "%s\n\n",       STR(flag->description))
    );
    return true;
}

bool MyArgvParse(MyArgvFlag** flags, size_t flagsc, const char* const* argv, int argc, void (*MyArgvUnkownFlagCallback)(const char*)) {
    if (argc == 0) { return false; } 
    MY_ASSERT_PTR(argv);

    qsort(flags, flagsc, sizeof(MyArgvFlag*), MyArgvQsortCmp);
    MyArgvFlag* shortNameJumpTable[256] = {0};
    for (size_t i = 0; i < flagsc; i++) {
        if (flags[i]->shortName == 0 || flags[i]->shortName == '~') { continue; }
        shortNameJumpTable[(unsigned char)flags[i]->shortName] = flags[i];
    }
    
    bool help = false;
    for (int i = 0; i < argc; i++) {
        const char* arg = argv[i];
        const char* next = (i + 1 < argc) ? argv[i + 1] : NULL;

        if (arg == NULL) { continue; }
        if (arg[0] != '-') { MyArgvUnkownFlagCallback(arg); continue; } // Ignoring anything that does not begin with '-'
        if (arg[1] == '\0') { MyArgvUnkownFlagCallback(arg); continue; } // Ignoring "-"
        if (strcmp(arg, "--help") == 0) { 
            MY_ASSERT(next != NULL && next[0] != '\0', "Correct usage: --help [flag] or --help [short flag]");
            if (!MyArgvParseHelp(shortNameJumpTable, flags, flagsc, next)) { MyArgvUnkownFlagCallback(arg); }
            help = true;
            i++;
            continue;
        }

        if (arg[1] == '-') {
            if (arg[2] == '\0') { MyArgvUnkownFlagCallback(arg); continue; } // Ignoring "--"
            if (!MyArgvParseLong(flags, flagsc, arg)) { MyArgvUnkownFlagCallback(arg); }
            continue;
        }

        if (!MyArgvParseShort(shortNameJumpTable, arg, next, &i)) { MyArgvUnkownFlagCallback(arg); }
    }
    return help;
}

#ifdef __cplusplus
}
#endif