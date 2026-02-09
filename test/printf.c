#define MYSTD_STDLIB_IMPLEMENTATION
#define MYSTD_STDIO_IMPLEMENTATION
#define MYSTD_PRINTF_IMPLEMENTATION
#include <mystd/printf.h>

#define TEST(expr) do { \
    if (!(expr)) { \
        MyPrintf("FAIL: %s (line %d)\n", #expr, __LINE__); \
    } else { \
        MyPrintf("PASS: %s\n", #expr); \
    } \
} while (0)

#define TEST_STR(actual, expected) do { \
    if (strcmp((actual), (expected)) != 0) { \
        MyPrintf("FAIL: \"%s\" != \"%s\" (line %d)\n", actual, expected, __LINE__); \
    } else { \
        MyPrintf("PASS: \"%s\"\n", expected); \
    } \
} while (0)

void Test_U32(void) {
    MyPrintf("\n==== MyU32tos ====\n");

    TEST_STR(MyU32tos(0, false, false), "0");
    TEST_STR(MyU32tos(1, false, false), "1");
    TEST_STR(MyU32tos(9, false, false), "9");
    TEST_STR(MyU32tos(10, false, false), "10");
    TEST_STR(MyU32tos(99, false, false), "99");
    TEST_STR(MyU32tos(100, false, false), "100");
    TEST_STR(MyU32tos(101, false, false), "101");

    TEST_STR(MyU32tos(4294967295u, false, false), "4294967295");
}
void Test_I32(void) {
    MyPrintf("\n==== MyI32tos ====\n");

    TEST_STR(MyI32tos(0, false, false), "0");
    TEST_STR(MyI32tos(0, true, false),  "+0");

    TEST_STR(MyI32tos(42, false, false), "42");
    TEST_STR(MyI32tos(42, true, false),  "+42");

    TEST_STR(MyI32tos(-1, false, false), "-1");
    TEST_STR(MyI32tos(-42, false, false), "-42");

    TEST_STR(MyI32tos(INT32_MIN, false, false), "-2147483648");
    TEST_STR(MyI32tos(INT32_MAX, false, false), "2147483647");
}
void Test_U64_I64(void) {
    MyPrintf("\n==== MyU64tos / MyI64tos ====\n");

    TEST_STR(MyU64tos(0, false, false), "0");
    TEST_STR(MyU64tos(18446744073709551615ull, false, false), "18446744073709551615");

    TEST_STR(MyI64tos(0, false, false), "0");
    TEST_STR(MyI64tos(0, true, false),  "+0");

    TEST_STR(MyI64tos(-1, false, false), "-1");
    TEST_STR(MyI64tos(INT64_MIN, false, false), "-9223372036854775808");
    TEST_STR(MyI64tos(INT64_MAX, false, false), "9223372036854775807");
}
void Test_F32(void) {
    MyPrintf("\n==== MyF32tos ====\n");

    TEST_STR(MyF32tos(0.0f, 2, false, false), "0.00");
    TEST_STR(MyF32tos(0.0f, 2, true, false),  "+0.00");

    TEST_STR(MyF32tos(1.0f, 0, false, false), "1");
    TEST_STR(MyF32tos(1.5f, 0, false, false), "2");   // rounding

    TEST_STR(MyF32tos(1.2f, 6, false, false), "1.200000");

    TEST_STR(MyF32tos(3.141596f, 2, false, false), "3.14");
    TEST_STR(MyF32tos(3.141596f, 4, false, false), "3.1416");

    TEST_STR(MyF32tos(-3.141596f, 3, false, false), "-3.142");

    TEST_STR(MyF32tos(0.00123f, 5, false, false), "0.00123");
    TEST_STR(MyF32tos(0.00001f, 6, false, false), "0.000010");

    TEST_STR(MyF32tos(9999999.0f, 2, false, false), "9999999.00");

    TEST_STR(MyF32tos(NAN, 2, false, false), "nan");
    TEST_STR(MyF32tos(INFINITY, 2, false, false), "inf");
    TEST_STR(MyF32tos(-INFINITY, 2, false, false), "-inf");
}
void Test_F64(void) {
    MyPrintf("\n==== MyF64tos ====\n");

    TEST_STR(MyF64tos(0.0, 2, false, false), "0.00");
    TEST_STR(MyF64tos(1.0, 0, false, false), "1");

    TEST_STR(MyF64tos(1.9999999999999, 2, false, false), "2.00");
    TEST_STR(MyF64tos(3.141592653589793, 6, false, false), "3.141593");

    TEST_STR(MyF64tos(-3.141592653589793, 4, false, false), "-3.1416");

    TEST_STR(MyF64tos(0.000000000000123, 15, false, false), "0.000000000000123");

    TEST_STR(MyF64tos(1234567890123.0, 0, false, false), "1234567890123");

    TEST_STR(MyF64tos(NAN, 3, false, false), "nan");
    TEST_STR(MyF64tos(INFINITY, 3, false, false), "inf");
    TEST_STR(MyF64tos(-INFINITY, 3, false, false), "-inf");
}

void Test_Stress(void) {
    MyPrintf("\n==== Stress ====\n");

    MyPrintf(
        "%s %s %s %s %s %s %s %s\n",
        MyU32tos(10, false, false),
        MyU32tos(20, false, false),
        MyI32tos(-30, false, false),
        MyF32tos(1.23f, 2, false, false),
        MyF64tos(4.5678, 3, false, false),
        MyU64tos(999999999999ull, false, false),
        MyI64tos(-888888888888ll, false, false),
        MyF32tos(0.001f, 5, false, false)
    );
}

int main(void) {
    Test_U32();
    Test_I32();
    Test_U64_I64();
    Test_F32();
    Test_F64();
    Test_Stress();

    char buffer[1024] = {0};

    MySnprintf(buffer, sizeof(buffer), "%s", "\nHola Mundo!");
    MyFilePrint(MyStdout(), buffer);

    MySnprintf(buffer, sizeof(buffer), "\nPI = %f", 3.141596f);
    MyFilePrint(MyStdout(), buffer);

    return 0;
}