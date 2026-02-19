#ifndef __MYSTD_STDLIB_H__
#define __MYSTD_STDLIB_H__

#include <mystd/stddef.h>

/* --------------------------------------------------------------------------
 * RAW
 * -------------------------------------------------------------------------- */

void MyRawOutput(const char* msg);
void MyRawError(const char* msg);

char* MyRawStrcpy(char* dst, const char* end, const char* src);
size_t MyRawSnprintf(char* dst, size_t max, const char* format, ...);

/* --------------------------------------------------------------------------
 * ASSERT
 * -------------------------------------------------------------------------- */

MY_NORETURN void MyExit();

void MyAssertLog(const char* msg, MyContext context);
void MyAssertBoundsLog(size_t idx, size_t bounds, MyContext context);

#define MY_ASSERT(cnd, msg)                      do { if (!(cnd))            { MyAssertLog(msg, MY_CONTEXT(NULL));                                                                  MyExit(); } } while(0)
#define MY_ASSERT_PTR(ptr)                       do { if ((ptr) == NULL)     { MyAssertLog("'"#ptr "' is NULL", MY_CONTEXT(NULL));                                                  MyExit(); } } while(0)
#define MY_ASSERT_BOUNDS(idx, bound)             do { if ((idx) >= (bound))  { MyAssertBoundsLog(idx, bound, MY_CONTEXT(NULL));                                                     MyExit(); } } while(0)
#define MY_ASSERT_MALLOC(ptr, type, size)        do { if (ptr == NULL)       { MyAssertLog("Malloc failed for "  #ptr " of type " #type " and size "  #size, MY_CONTEXT(NULL));     MyExit(); } } while(0)
#define MY_ASSERT_CALLOC(ptr, type, count)       do { if (ptr == NULL)       { MyAssertLog("Calloc failed for "  #ptr " of type " #type " and count " #count, MY_CONTEXT(NULL));    MyExit(); } } while(0)
#define MY_ASSERT_REALLOC(ptr, type, size)       do { if (ptr == NULL)       { MyAssertLog("Realloc failed for " #ptr " of type " #type " and size "  #size, MY_CONTEXT(NULL));     MyExit(); } } while(0)

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
const char* MyPtrdifftos(ptrdiff_t value);

void MyNormalizePath(char* path);

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

#endif /* __MYSTD_STDLIB_H__ */