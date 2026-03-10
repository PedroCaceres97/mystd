#include <mystd/argv-parser.h>

MY_RWLOCK_DEFINES(MyArgvParser, parser, MyArgvParser)

MyArgvParser*   MyArgvParser_Create     (MyArgvParser* parser) {
    MY_ADOPT_OR_ALLOC(parser, MyArgvParser);
    return parser;
}
void            MyArgvParser_Destroy    (MyArgvParser* parser) {
    MY_ASSERT_PTR(parser);
    MY_FREE_ADOPTED(parser);
}
void            MyArgvParser_Register   (MyArgvParser* parser, char short_name, const char* long_name, bool8 expect_value, void (*handler)(const char* value)) {
    MY_ASSERT_PTR(parser);
    MY_ASSERT_BOUNDS(parser->flags_count, MY_ARGV_PARSER_FLAGS_COUNT);
    parser->flags[parser->flags_count].handler      = handler;
    parser->flags[parser->flags_count].short_name   = short_name;
    parser->flags[parser->flags_count].long_name    = long_name;
    parser->flags[parser->flags_count].expect_value = expect_value;
    parser->flags_count++;
}
void            MyArgvParser_Parse      (MyArgvParser* parser, const char** start_argv, int argc) {
    for (int i = 0; i < argc; i++) {
        MY_ASSERT(start_argv[i][0] == '-', "Founded an argument non starting with '-' without matching flag");
        const char* start = &start_argv[i][2];

        for (int j = 0; j < parser->flags_count; j++) {
            const char* value = NULL;

            if (start_argv[i][1] != '-') {
                goto short_flag;
            }

        long_flag:
            if (parser->flags[j].long_name == NULL) { continue; }

            if (strncmp(start, parser->flags[j].long_name, strlen(parser->flags[j].long_name)) != 0) {
                continue;
            }

            if (parser->flags[j].expect_value) {
                const char* equal = strchr(start, '=');
                MY_ASSERT(equal && start != equal && equal[1] != '\0', "Any long form flag that requires a value must be preceeded by '=' and their value -> --[flag]=[value]");
                value = &equal[1];
            }
            parser->flags[j].handler(value);
            break;

        short_flag:
            if (parser->flags[j].short_name == 0 || start_argv[i][1] != parser->flags[j].short_name) {
                continue;
            }

            if (parser->flags[j].expect_value) {
                if (start_argv[i][2] != '\0') { value = &start_argv[i][2]; }
                else {
                    MY_ASSERT(i + 1 < argc, "Any short form flag that requires a value must be preceeded by their value -> -[flag][value] o -[flag] [value]");
                    value = start_argv[++i];
                }
            }

            parser->flags[j].handler(value);
            break;
        }
    }
}