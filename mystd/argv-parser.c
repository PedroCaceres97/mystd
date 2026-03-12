#include <mystd/argv-parser.h>

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
void MyArgvParser_Parse(MyArgvParser* parser, const char** argv, int argc) {
    for (int i = 0; i < argc; i++) {

        const char* arg = argv[i];

        if (arg[0] != '-') { continue; }

        size_t arg_len = strlen(arg);
        if (arg_len < 2) { continue; } // Ignoring just "-" args

        bool8 is_long = (arg[1] == '-');
        const char* name = is_long ? &arg[2] : &arg[1];

        for (int j = 0; j < parser->flags_count; j++) {
            MyArgvParserFlag* flag = parser->flags[j];
            const char* value = NULL;

            if (is_long) { goto long_flag; } 
        
        short_flag:
            if (flag->short_name ==0 || name[0] != flag->short_name) { continue; }

            if (flag->expect_value) {
                if (name[1] != '\0') { value = &name[1]; } 
                else {
                    MY_ASSERT(i + 1 < argc, "Any short form flag that requires a value must be preceeded by their value -> -[flag][value] o -[flag] [value]");
                    value = argv[++i];
                }
            }
            goto founded_flag;

        long_flag:
            if (!flag->long_name) { continue; }

            size_t name_len = strlen(flag->long_name);
            
            if (strncmp(name, flag->long_name, name_len) != 0) { continue; }

            if (flag->expect_value) {
                MY_ASSERT(arg_len >= name_len + 4 && name[name_len] == '=', "Any long form flag that requires a value must be preceeded by '=' and their value -> --[flag]=[value]");
                value = &name[name_len + 1];
            }

        founded_flag:
            flag->listener = true;

            if (value) {
                strncpy(flag->value, value, MY_ARGV_PARSER_VALUE_SIZE - 1);
                flag->value[MY_ARGV_PARSER_VALUE_SIZE - 1] = '\0';
            } else {
                flag->value[0] = '\0';
            }

            break;
        }
    }
}