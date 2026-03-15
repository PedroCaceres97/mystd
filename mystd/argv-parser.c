#include <mystd/argv-parser.h>

static int MyArgvParser_QsortCmp(const void* a, const void* b) {
    const MyArgvParserFlag* f1 = *(const MyArgvParserFlag* const*)a;
    const MyArgvParserFlag* f2 = *(const MyArgvParserFlag* const*)b;

    const char* s1 = f1->long_name ? f1->long_name : "";
    const char* s2 = f2->long_name ? f2->long_name : "";

    return strcmp(s1, s2);
}
static int MyArgvParser_BsearchCmp(const void* key, const void* elem) {
    const char* str = key;
    const MyArgvParserFlag* flag = *(const MyArgvParserFlag* const*)elem;

    const char* name = flag->long_name ? flag->long_name : "";
    return strcmp(str, name);
}

static bool MyArgvParser_ParseShort(MyArgvParser* parser, const char* arg, const char* next) { 
    /*
        At this point is it guaranteed that arg starts with '-' and
        has at least another characther following it.
    */
    MyArgvParserFlag* flag = parser->short_name_jump_table[(unsigned char)arg[1]];
    if (flag == NULL) { return false; } // -1 indicates that arg[1] characther was not registered

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
static void MyArgvParser_ParseLong(MyArgvParser* parser, const char* arg) {
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
    MyArgvParserFlag** pflag = bsearch(key, parser->flags, parser->flags_count, sizeof(MyArgvParserFlag*), MyArgvParser_BsearchCmp);
    if (pflag == NULL) { return; }

    MyArgvParserFlag* flag = *pflag;
    if (flag->long_name == NULL) { return; }
    size_t len = strlen(flag->long_name);
    flag->listener = true;

    if (flag->expect_value) {
        MY_ASSERT(arg[len + 2] == '=' && arg[len + 3] != '\0', "Any long form flag that requires a value must be preceeded by '=' and their value -> --[flag]=[value]"); 
        strncpy(flag->value, &arg[len + 3], MY_ARGV_PARSER_VALUE_SIZE - 1);
        flag->value[MY_ARGV_PARSER_VALUE_SIZE - 1] = '\0';
    }
}

MY_RWLOCK_DEFINES(MyArgvParser, parser, MyArgvParser)

MyArgvParser*   MyArgvParser_Create     (MyArgvParser* parser) {
    MY_STRUCT_CREATE_RULE(parser, MyArgvParser);
    for (int i = 0; i < 256; i++) {
        parser->short_name_jump_table[i] = NULL;
    }
    return parser;
}
void            MyArgvParser_Destroy    (MyArgvParser* parser) {
    MY_ASSERT_PTR(parser);
    MY_STRUCT_DESTROY_RULE(parser);
}
void            MyArgvParser_Register   (MyArgvParser* parser, MyArgvParserFlag* flag) {
    MY_ASSERT_PTR(parser);
    MY_ASSERT_BOUNDS(parser->flags_count, MY_ARGV_PARSER_FLAGS_COUNT);
    MY_ASSERT(parser->short_name_jump_table[(unsigned char)flag->short_name] == NULL, MySprintf("A flag with '%c' as a short name was already registered", flag->short_name));
    // A value of zero in short_name should be ignored and allowed to be registered multiple times
    if (flag->short_name != 0) { parser->short_name_jump_table[(unsigned char)flag->short_name] = flag; }
    parser->flags[parser->flags_count] = flag;
    parser->flags_count++;
    parser->sorted = false;
}
void            MyArgvParser_Parse      (MyArgvParser* parser, const char** argv, int argc) {
    if (argc == 0) { return; } 
    MY_ASSERT_PTR(argv);

    if (!parser->sorted) {
        qsort(parser->flags, parser->flags_count, sizeof(MyArgvParserFlag*), MyArgvParser_QsortCmp);
        parser->sorted = true;
    }
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