#ifndef __MYSTD_ARGV_PARSER_H__
#define __MYSTD_ARGV_PARSER_H__

#include <mystd/stdio.h>

#ifndef MY_ARGV_PARSER_FLAGS_COUNT
    #define MY_ARGV_PARSER_FLAGS_COUNT 128
#endif /* MY_ARGV_PARSER_FLAGS_COUNT */

#ifndef MY_ARGV_PARSER_VALUE_SIZE
    #define MY_ARGV_PARSER_VALUE_SIZE 128
#endif /* MY_ARGV_PARSER_VALUE_SIZE */

typedef struct MyArgvParserFlag {
    const char*     long_name;
    bool8           listener;
    char            value[MY_ARGV_PARSER_VALUE_SIZE]; 
    char            short_name;
    bool8           expect_value;
} MyArgvParserFlag;

typedef struct MyArgvParser {
    MyStructHeader      header;

    MyArgvParserFlag*   flags[MY_ARGV_PARSER_FLAGS_COUNT];
    int                 flags_count;
} MyArgvParser;

MY_RWLOCK_DECLARES(MyArgvParser, parser, MyArgvParser)

/* start_argv must be the start to the flags to parse */
MyArgvParser*   MyArgvParser_Create     (MyArgvParser* parser);
void            MyArgvParser_Destroy    (MyArgvParser* parser);
void            MyArgvParser_Register   (MyArgvParser* parser, MyArgvParserFlag* flag);
void            MyArgvParser_Parse      (MyArgvParser* parser, const char** argv, int argc);

#endif /* __MYSTD_ARGV_PARSER_H__ */