#include "stdlib.h"
#include <mystd/stdlib.h>

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

static void MyAnsi_Parse(MyPrintfSpec* spec, MyBuffer* buffer, MyFormat* format) {
    if (!format->text) { return; }
    
    while (*format->text) {
        char key = *format->text++;
        void (*parser)(MyPrintfSpec*, MyFormat*) = myAnsiParsers[key];
        if (parser == NULL) { continue; }
        MyFormat_AdvanceIfEq(format, ':');
        MyFormat_AdvanceIfEq(format, ' ');
        parser(spec, format);
        MyBuffer_Write(buffer, spec->data);
        if (myAnsiParsersReset[key]) { MyBuffer_Write(buffer, MY_ANSI_RESET); }
    }
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

    if (buf.data) { *buf.data = '\0'; }
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
size_t MySnprintfSegmentsN(char* buffer, size_t max, MyPrintfSegment* segments, size_t count) {
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
        fmt.text = segments[i].format;

        MyAnsi_Parse(&spec, &buf, &fmt);

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
    }

    if (buf.data) { *buf.data = '\0'; }
    return buf.written;
}