#include <__stdarg_va_list.h>
#include <mystd/stdlib.h>

#if defined(MY_OS_WINDOWS)
    #include <mystd/stdwin.h>

    void MyWindowsPrintLastError() {
        LPVOID lpMsgBuf;
        DWORD dw = GetLastError(); 
        MY_ASSERT(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL), "Failed to format windows error");
        char buffer[512] = {0};
        MyRawSnprintf(buffer, sizeof(buffer), "\nWindows error %u: %s\n\n", (unsigned int)dw, lpMsgBuf);
        MyRawError(buffer);
        LocalFree(lpMsgBuf);
    }
#elif defined(MY_OS_LINUX)

#endif

/* --------------------------------------------------------------------------
 * RAW
 * -------------------------------------------------------------------------- */

void MyRawOutput(const char* msg) {
#if defined(MY_OS_WINDOWS)
    DWORD written = 0;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), msg, strlen(msg), &written, NULL);
#elif defined(MY_OS_LINUX)

#endif
}
void MyRawError(const char* msg) {
#if defined(MY_OS_WINDOWS)
    DWORD written = 0;
    WriteFile(GetStdHandle(STD_ERROR_HANDLE), msg, strlen(msg), &written, NULL);
#elif defined(MY_OS_LINUX)
    
#endif
}

char* MyRawStrcpy(char* dst, const char* end, const char* src) {
    while (*src && dst < end) { *dst++ = *src++; }
    return dst;
}

void MyRawSnprintf(char* dst, size_t size, const char* format, ...) {
    if (!dst || !format || size == 0) return;
    va_list args;
    va_start(args, format);

    char ch = 0;
    const char* str= NULL;
    size_t remaining = size;
    while (*format && remaining > 1) {
        if (*format != '%') {
            ch = *format;
            goto write_char;
        }

        format++;
        if (*format == '%') {
            ch = '%';
            goto write_char;
        } else if (*format == 'c') {
            ch = (char)va_arg(args, int);
            goto write_char;
        } else if (*format == 's') {
            str = va_arg(args, const char*);
            if (!str) str = "(null)";
            goto write_string;
        } else if (*format == 'i') {
            int n = va_arg(args, int);
            str = MyI32tos(n, false, false);
            goto write_string;
        } else if (*format == 'u') {
            unsigned int n = va_arg(args, unsigned int);
            str = MyU32tos(n, false, false);
            goto write_string;
        } else if (*format == 'z') {
            size_t n = va_arg(args, size_t);
            str = MySizetos(n);
            goto write_string;
        } else if (*format == 'f') {
            double n = va_arg(args, double);
            str = MyF64tos(n, 6, false, false);
            goto write_string;
        }

    write_char:
        *dst++ = ch;
        remaining--;
        format++;
        continue;

    write_string:
        while (*str && remaining > 1) { *dst++ = *str++; remaining--; }
        format++;
        continue;
    }

    *dst = '\0';
    va_end(args);
}

/* --------------------------------------------------------------------------
 * ASSERT
 * -------------------------------------------------------------------------- */

MY_NORETURN void MyExit() {
  #ifndef ABORT_ON_ERROR
    exit(-1);
  #else
    abort();
  #endif
}

void MyAssertLog(const char* msg, MyContext context) {
    char buffer[1024] = {0};
    MyRawSnprintf(buffer, sizeof(buffer), "[MY_ASSERT FAILED]:\n Context: %s:%u (%s)\n Message: %s\n\n", context.file, context.line, context.func, msg);
    MyRawError(buffer);
}
void MyAssertBoundsLog(size_t idx, size_t bounds, MyContext context) {
    char buffer[1024] = {0};
    MyRawSnprintf(buffer, sizeof(buffer), "[MY_ASSERT FAILED]:\n Context: %s:%u (%s)\n Message: Index (%z) out of bounds (%z)\n\n", context.file, context.line, context.func, idx, bounds);
    MyRawError(buffer);
}

/* --------------------------------------------------------------------------
 * ANSI
 * -------------------------------------------------------------------------- */

static thread_local char myAnsiBuffers[MY_ANSI_BUFFERS][MY_ANSI_BUFFER_SIZE];
static thread_local uint32_t myAnsiIndex = 0;

static inline char* MyAnsiNextBuffer() {
    myAnsiIndex++;
    if (myAnsiIndex == MY_ANSI_BUFFERS) { myAnsiIndex = 0; }
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
    return buffer;
}

char* MyAnsiCursorUp(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'A';
    return buffer;
}
char* MyAnsiCursorDown(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'B';
    return buffer;
}
char* MyAnsiCursorForward(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'C';
    return buffer;
}
char* MyAnsiCursorBack(uint16_t n) {
    char* buffer = MyAnsiNextBuffer();
    *buffer++ = '\x1b';
    *buffer++ = '[';

    buffer = MyU32tos_((uint32_t)n, buffer);

    *buffer++ = 'D';
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
    return buffer;
}

/* --------------------------------------------------------------------------
 * To String
 * -------------------------------------------------------------------------- */

static thread_local char myTosBuffers[MY_TOS_BUFFERS][MY_TOS_BUFFER_SIZE];
static thread_local uint32_t myTosIndex = 0;

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

static inline char* MyTosNextBuffer() {
    myTosIndex++;
    if (myTosIndex == MY_TOS_BUFFERS) { myTosIndex = 0; }
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
    return MyX32tos((uint32_t)(uintptr_t)value);
#elif defined(MY_PTR_64BIT)
    return MyX64tos((uint64_t)(uintptr_t)value);
#else
    MY_ASSERT(false, "Unsupported pointer size");
#endif
}
const char* MySizetos(size_t value) {
#if defined(MY_SIZE_32BIT)
    return MyU32tos((uint32_t)(uintptr_t)value, false, false);
#elif defined(MY_SIZE_64BIT)
    return MyU64tos((uint64_t)(uintptr_t)value, false, false);
#else
    MY_ASSERT(false, "Unsupported size_t size");
#endif
}

void MyNormalizePath(char* path) {
    MY_ASSERT_PTR(path);
#ifdef MY_OS_WINDOWS
    while (*path) { if (*path == '/') { *path = '\\'; } path++; }
#else
    while (*path) { if (*path == '\\') { *path = '/'; } path++; }
#endif
}