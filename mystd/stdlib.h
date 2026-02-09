#ifndef __MYSTD_STDLIB_H__
#define __MYSTD_STDLIB_H__

#include <mystd/stddef.h>

/* --------------------------------------------------------------------------
 * ASSERT
 * -------------------------------------------------------------------------- */

MY_NORETURN void MyExit();

void MyRawOutput(const char* msg);
void MyRawError(const char* msg);

void MyAssertEmit(const char* msg, MyContext context);
void MyAssertBoundsEmit(size_t idx, size_t bounds, MyContext context);

char* MyRawStrcpy(char* dst, const char* end, const char* src);

#define MY_ASSERT(cnd, msg)                      do { if (!(cnd))            { MyAssertEmit(msg, MY_CONTEXT(NULL));                                                                 MyExit(); } } while(0)
#define MY_ASSERT_PTR(ptr)                       do { if ((ptr) == NULL)     { MyAssertEmit("'"#ptr "' is NULL", MY_CONTEXT(NULL));                                                 MyExit(); } } while(0)
#define MY_ASSERT_BOUNDS(idx, bound)             do { if ((idx) >= (bound))  { MyAssertBoundsEmit(idx, bound, MY_CONTEXT(NULL));                                                    MyExit(); } } while(0)
#define MY_ASSERT_MALLOC(ptr, type, size)        do { if (ptr == NULL)       { MyAssertEmit("Malloc failed for "  #ptr " of type " #type " and size "  #size, MY_CONTEXT(NULL));    MyExit(); } } while(0)
#define MY_ASSERT_CALLOC(ptr, type, count)       do { if (ptr == NULL)       { MyAssertEmit("Calloc failed for "  #ptr " of type " #type " and count " #count, MY_CONTEXT(NULL));   MyExit(); } } while(0)
#define MY_ASSERT_REALLOC(ptr, type, size)       do { if (ptr == NULL)       { MyAssertEmit("Realloc failed for " #ptr " of type " #type " and size "  #size, MY_CONTEXT(NULL));    MyExit(); } } while(0)

#ifndef MY_EMPTY_POPPING_ENABLE
    #define MY_EMPTY_POPPING() MY_ASSERT(false, "To enable empty popping and avoid this error define MY_EMPTY_POPPING_ENABLE")
#endif /* MY_EMPTY_POPPING_ENABLE */

#if defined(MY_ASSERT_DISABLE) || defined(MY_ASSERT_DISABLE_ALL)
  #undef  MY_ASSERT
  #undef  MY_ASSERTF
  #define MY_ASSERT(cnd, msg)            do { ((void)0) } while(0)
  #define MY_ASSERTF(cnd, format, ...)   do { ((void)0) } while(0)
#endif

#if defined(MY_ASSERT_DISABLE_PTR) || defined(MY_ASSERT_DISABLE_ALL)
  #undef  MY_ASSERT_PTR
  #define MY_ASSERT_PTR(ptr)             do { ((void)0) } while(0)
#endif

#if defined(MY_ASSERT_DISABLE_BOUNDS) || defined(MY_ASSERT_DISABLE_ALL)
  #undef  MY_ASSERT_BOUNDS
  #define MY_ASSERT_BOUNDS(idx, bound)   do { ((void)0) } while(0)
#endif

/* --------------------------------------------------------------------------
 * ANSI
 * -------------------------------------------------------------------------- */

#ifndef MY_ANSI_BUFFERS
    #define MY_ANSI_BUFFERS 16
#endif /* MY_ANSI_BUFFERS */
#ifndef MY_ANSI_BUFFER_SIZE
    #define MY_ANSI_BUFFER_SIZE 64
#endif /* MY_ANSI_BUFFER_SIZE */

#define MY_ANSI_ESC  "\x1b["

#define MY_ANSI_COLOR(color, text)  color text MY_ANSI_RESET

#define MY_ANSI_FG_BLACK           MY_ANSI_ESC "30m"
#define MY_ANSI_FG_RED             MY_ANSI_ESC "31m"
#define MY_ANSI_FG_GREEN           MY_ANSI_ESC "32m"
#define MY_ANSI_FG_YELLOW          MY_ANSI_ESC "33m"
#define MY_ANSI_FG_BLUE            MY_ANSI_ESC "34m"
#define MY_ANSI_FG_MAGENTA         MY_ANSI_ESC "35m"
#define MY_ANSI_FG_CYAN            MY_ANSI_ESC "36m"
#define MY_ANSI_FG_WHITE           MY_ANSI_ESC "37m"

#define MY_ANSI_BG_BLACK           MY_ANSI_ESC "40m"
#define MY_ANSI_BG_RED             MY_ANSI_ESC "41m"
#define MY_ANSI_BG_GREEN           MY_ANSI_ESC "42m"
#define MY_ANSI_BG_YELLOW          MY_ANSI_ESC "43m"
#define MY_ANSI_BG_BLUE            MY_ANSI_ESC "44m"
#define MY_ANSI_BG_MAGENTA         MY_ANSI_ESC "45m"
#define MY_ANSI_BG_CYAN            MY_ANSI_ESC "46m"
#define MY_ANSI_BG_WHITE           MY_ANSI_ESC "47m"

#define MY_ANSI_RESET              MY_ANSI_ESC "0m"
#define MY_ANSI_BOLD               MY_ANSI_ESC "1m"
#define MY_ANSI_DIM                MY_ANSI_ESC "2m"
#define MY_ANSI_ITALIC             MY_ANSI_ESC "3m"
#define MY_ANSI_UNDERLINE          MY_ANSI_ESC "4m"
#define MY_ANSI_BLINK              MY_ANSI_ESC "5m"
#define MY_ANSI_REVERSE            MY_ANSI_ESC "7m"
#define MY_ANSI_HIDDEN             MY_ANSI_ESC "8m"
#define MY_ANSI_STRIKETHROUGH      MY_ANSI_ESC "9m"
#define MY_ANSI_DOUBLE_UNDER       MY_ANSI_ESC "21m"
#define MY_ANSI_OVERLINE           MY_ANSI_ESC "53m"
#define MY_ANSI_RESET_TERMINAL     MY_ANSI_ESC "c"

#define MY_ANSI_FG_256(n)          MY_ANSI_ESC "38;5;" #n "m"
#define MY_ANSI_BG_256(n)          MY_ANSI_ESC "48;5;" #n "m"
#define MY_ANSI_FG_RGB(r, g, b)    MY_ANSI_ESC "38;2;" #r ";" #g ";" #b "m"
#define MY_ANSI_BG_RGB(r, g, b)    MY_ANSI_ESC "48;2;" #r ";" #g ";" #b "m"

#define MY_ANSI_CURSOR_UP(n)       MY_ANSI_ESC #n "A"
#define MY_ANSI_CURSOR_DOWN(n)     MY_ANSI_ESC #n "B"
#define MY_ANSI_CURSOR_FORWARD(n)  MY_ANSI_ESC #n "C"
#define MY_ANSI_CURSOR_BACK(n)     MY_ANSI_ESC #n "D"
#define MY_ANSI_CURSOR_POS(x, y)   MY_ANSI_ESC #y ";" #x "H"
#define MY_ANSI_CURSOR_HOME        MY_ANSI_ESC "H"

char* MyAnsiFg256        (uint8_t n);
char* MyAnsiBg256        (uint8_t n);
char* MyAnsiFgRGB        (uint8_t r, uint8_t g, uint8_t b);
char* MyAnsiBgRGB        (uint8_t r, uint8_t g, uint8_t b);

char* MyAnsiCursorUp     (uint16_t n);
char* MyAnsiCursorDown   (uint16_t n);
char* MyAnsiCursorForward(uint16_t n);
char* MyAnsiCursorBack   (uint16_t n);
char* MyAnsiCursorPos    (uint16_t x, uint16_t y);

#define ANSI_CURSOR_SAVE        ANSI_ESC "s"
#define ANSI_CURSOR_RESTORE     ANSI_ESC "u"
#define ANSI_CURSOR_HIDE        ANSI_ESC "?25l"
#define ANSI_CURSOR_SHOW        ANSI_ESC "?25h"

#define ANSI_CLEAR_SCREEN       ANSI_ESC "2J"
#define ANSI_CLEAR_LINE         ANSI_ESC "2K"
#define ANSI_CLEAR_TO_END       ANSI_ESC "0J"
#define ANSI_CLEAR_TO_START     ANSI_ESC "1J"
#define ANSI_CLEAR_LINE_END     ANSI_ESC "0K"
#define ANSI_CLEAR_LINE_START   ANSI_ESC "1K"

/* --------------------------------------------------------------------------
 * To String
 * -------------------------------------------------------------------------- */

#ifndef MY_TOS_BUFFERS
    #define MY_TOS_BUFFERS 16
#endif /* MY_ANSI_BUFFERS */
#ifndef MY_TOS_BUFFER_SIZE
    #define MY_TOS_BUFFER_SIZE 64
#endif /* MY_TOS_BUFFER_SIZE */

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

/* --------------------------------------------------------------------------
 * WRAPPERS
 * -------------------------------------------------------------------------- */

#define MY_STR_IMPL(x) #x
#define MY_CONCAT2_IMPL(a, b) a##b
#define MY_CONCAT3_IMPL(a, b, c) a##b##c

#define MY_STR(x) MY_STR_IMPL(x)
#define MY_CONCAT2(a, b) MY_CONCAT2_IMPL(a, b)
#define MY_CONCAT3(a, b, c) MY_CONCAT3_IMPL(a, b, c)

#define MY_FREE(ptr)                   do { free((void*)(ptr)); (ptr) = NULL; } while(0)
#define MY_FREE_IF(ptr)                do { if ((ptr)) { MY_FREE((ptr)); }  } while(0)
#define MY_MALLOC(v, type, size)       do { (v) = (type*)malloc((size));                MY_ASSERT_MALLOC((v), type, (size)); memset(v, 0, (size)); } while(0)
#define MY_CALLOC(v, type, count)      do { (v) = (type*)calloc((count), sizeof(type)); MY_ASSERT_CALLOC((v), type, (count));                      } while(0)
#define MY_REALLOC(v, type, ptr, size) do { (v) = (type*)realloc((void*)(ptr), (size)); MY_ASSERT_REALLOC((v), type, (size));                      } while(0)

#define MY_CAST(type, tocast)   ((type)(tocast))
#define MY_TERNARY(cnd, x, y)   ((cnd) ? (x) : (y))

#define MY_MIN(x, y)            (MY_TERNARY(x > y, y, x))
#define MY_MAX(x, y)            (MY_TERNARY(x > y, x, y))
#define MY_CLAMP(x, min, max)   (MY_TERNARY(x < min, min, MY_TERNARY(x > max, max, x)))
#define MY_DISTANCE(x, y)       (MY_TERNARY(x > y, x - y, y - x))

#define MY_PTR_ADD(ptr, value)  ((void*)((uint8_t*)(ptr) + (value)))
#define MY_PTR_SUB(ptr, value)  ((void*)((uint8_t*)(ptr) - (value)))

#ifdef MY_STDLIB_IMPLEMENTATION

#if defined(MY_OS_WINDOWS)
    #include <windows.h>
#elif defined(MY_OS_LINUX)

#endif

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

void MyAssertEmit(const char* msg, MyContext context) {
    char buffer[1024] = {0};
    char* cursor = buffer;
    char* end = buffer + 1024;
    cursor = MyRawStrcpy(cursor, end, "[MY_ASSERT FAILED]:\n Context: ");
    cursor = MyRawStrcpy(cursor, end, context.file);
    cursor = MyRawStrcpy(cursor, end, ":");
    cursor = MyU32tos_(cursor, context.line);
    cursor = MyRawStrcpy(cursor, end, " (");
    cursor = MyRawStrcpy(cursor, end, context.func);
    cursor = MyRawStrcpy(cursor, end, ")\n Message: ");
    cursor = MyRawStrcpy(cursor, end, msg);
    cursor = MyRawStrcpy(cursor, end, "\n\n");
}
void MyAssertBoundsEmit(size_t idx, size_t bounds, MyContext context) {
    char buffer[1024] = {0};
    char* cursor = buffer;
    char* end = buffer + 1024;
    cursor = MyRawStrcpy(cursor, end, "[MY_ASSERT FAILED]:\n Context: ");
    cursor = MyRawStrcpy(cursor, end, context.file);
    cursor = MyRawStrcpy(cursor, end, ":");
    cursor = MyU32tos_(cursor, context.line);
    cursor = MyRawStrcpy(cursor, end, " (");
    cursor = MyRawStrcpy(cursor, end, context.func);
    cursor = MyRawStrcpy(cursor, end, ")\n Message: Index (");
    cursor = MyRawStrcpy(cursor, end, MySizetos(idx));
    cursor = MyRawStrcpy(cursor, end, ") out of bounds (");
    cursor = MyRawStrcpy(cursor, end, MySizetos(bounds));
    cursor = MyRawStrcpy(cursor, end, ")\n\n");
}

char* MyRawStrcpy(char* dst, const char* end, const char* src) {
    while (*src && dst < end) { *dst++ = *src++; }
    return dst;
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

static const char MY_DIGITS[200] =
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

static const char MY_HEX_PAIR[512] =
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
    MY_ASSERT(false, "Unsupported pointer size");
#endif
}

#endif /* MY_STDLIB_IMPLEMENTATION */

#endif /* __MYSTD_STDLIB_H__ */