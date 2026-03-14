#include <mystd/argv-parser.h>

static bool MyArgvParser_ParseShort(MyArgvParser* parser, const char* arg, const char* next) {
    /*
        At this point is it guaranteed that arg starts with '-' and
        has at least another characther following it.
    */
    for (int i = 0; i < parser->flags_count; i++) {
        MyArgvParserFlag* flag = parser->flags[i];
        if (flag->short_name == '\0' || flag->short_name != arg[1]) { continue; }
        flag->listener = true;

        if (flag->expect_value) {
            // As arg is guaranteed to have at least two characther we can check for null terminator in index 2 to check wheter value preceeds flag or is in 'next'
            if (arg[2] != '\0') { 
                strncpy(flag->value, &arg[2], MY_ARGV_PARSER_VALUE_SIZE - 1); 
                flag->value[MY_ARGV_PARSER_VALUE_SIZE - 1] = '\0';
                return false;
            } else { 
                MY_ASSERT(next != NULL, "Any short form flag that requires a value must be preceeded by their value -> -[flag][value] o -[flag] [value]"); 
                strncpy(flag->value, next, MY_ARGV_PARSER_VALUE_SIZE - 1); 
                flag->value[MY_ARGV_PARSER_VALUE_SIZE - 1] = '\0';
                return true;
            }
        }
        return false;
    }
    return false;
}

static void MyArgvParser_ParseLong(MyArgvParser* parser, const char* arg) {
    /*
        At this point is it guaranteed that arg starts with "--" and
        has at least another characther following it.
    */
    for (int i = 0; i < parser->flags_count; i++) {
        MyArgvParserFlag* flag = parser->flags[i];
        if (flag->long_name == NULL) { continue; }
        size_t len = strlen(flag->long_name);
        if (strncmp(flag->long_name, &arg[2], len) != 0) { continue; }
        if (arg[len + 2] != '=' && arg[len + 2] != '\0') { continue; } // Ignores misspelling
        flag->listener = true;

        if (flag->expect_value) {
            MY_ASSERT(arg[len + 2] == '=' && arg[len + 3] != '\0', "Any long form flag that requires a value must be preceeded by '=' and their value -> --[flag]=[value]"); 
            strncpy(flag->value, &arg[len + 3], MY_ARGV_PARSER_VALUE_SIZE - 1);
            flag->value[MY_ARGV_PARSER_VALUE_SIZE - 1] = '\0';
        }

        return;
    }
}

MY_RWLOCK_DEFINES(MyArgvParser, parser, MyArgvParser)

MyArgvParser*   MyArgvParser_Create     (MyArgvParser* parser) {
    MY_STRUCT_CREATE_RULE(parser, MyArgvParser);
    return parser;
}
void            MyArgvParser_Destroy    (MyArgvParser* parser) {
    MY_ASSERT_PTR(parser);
    MY_STRUCT_DESTROY_RULE(parser);
}
void            MyArgvParser_Register   (MyArgvParser* parser, MyArgvParserFlag* flag) {
    MY_ASSERT_PTR(parser);
    MY_ASSERT_BOUNDS(parser->flags_count, MY_ARGV_PARSER_FLAGS_COUNT);
    parser->flags[parser->flags_count] = flag;
    parser->flags_count++;
}
void            MyArgvParser_Parse      (MyArgvParser* parser, const char** argv, int argc) {
    if (argc == 0) { return; } 
    MY_ASSERT_PTR(argv);

    for (int i = 0; i < argc; i++) {
        const char* arg = argv[i];
        const char* next = (i + 1 < argc) ? argv[i + 1] : NULL;

        if (arg == NULL) { continue; }
        if (arg[0] != '-') { continue; } // Ignoring anything that does not begin with '-'
        if (arg[1] == '\0') { continue; } // Ignoring "-"

        if (arg[1] == '-') {
            if (arg[2] == '\0') { continue; } // Ignoring "--"
            MyArgvParser_ParseLong(parser, arg);
            continue;
        }

        if (MyArgvParser_ParseShort(parser, arg, next)) {
            i++;
        }
    }
}