#include "mystd/stdlib.h"
#define MY_JSON_VECTOR
#include <mystd/json.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t ln;
    size_t col;
    const char* source;
} MyJsonWriter;

#define MY_JSON_ASSERT(writer, cnd, msg) do { if (!(cnd)) { MyLog(MY_ERROR, "JSON Syntax Error at %zu:%zu", writer->ln, writer->col); MyFilePrint(MyStdout(), "Source JSON: \n"); MyFilePrint(MyStdout(), writer->source); MyFilePrint(MyStdout(), "\n\n"); MyLog(MY_FATAL, msg); } } while(0)

MY_RWLOCK_DEFINES(MyJson, json, MyJson)

/* ============================================================
   Helpers Declaration
   ============================================================ */

static MyJson* MyJson_Create(MyJsonType type, const char* key);
static void MyJson_Destroy(MyJson* json);
static void MyJson_Erase(MyJson* json);
static MyJson* MyJson_Add(MyJson* parent, MyJsonType type, const char* key);
static MyJson* MyJson_Find(MyJson* object, const char* key);

static void MyJson_NullReplace(MyJson* json);
static void MyJson_BoolReplace(MyJson* json, bool boolean);
static void MyJson_IntegerReplace(MyJson* json, int64_t integer);
static void MyJson_DecimalReplace(MyJson* json, double decimal);
static void MyJson_StringReplace(MyJson* json, const char* string);
static void MyJson_ArrayReplace(MyJson* json);
static void MyJson_ObjectReplace(MyJson* json);

static void MyJson_Print(MyJson* json, MyString* str, bool pretty, size_t indent);
static void MyJson_PrintKey(MyJson* json, MyString* str);
static void MyJson_PrintArray(MyJson* json, MyString* str, bool pretty, size_t indent);
static void MyJson_PrintObject(MyJson* json, MyString* str, bool pretty, size_t indent);

static void MyJson_Ignore(MyJsonWriter* writer);
static char* MyJson_ParseLiteral(MyJsonWriter* writer);

static MyJson* MyJson_Parse(MyJsonWriter* writer);
static MyJson* MyJson_ParseNull(MyJsonWriter* writer);
static MyJson* MyJson_ParseBool(MyJsonWriter* writer);
static MyJson* MyJson_ParseIntegerDecimal(MyJsonWriter* writer);
static MyJson* MyJson_ParseString(MyJsonWriter* writer);
static MyJson* MyJson_ParseArray(MyJsonWriter* writer);
static MyJson* MyJson_ParseObject(MyJsonWriter* writer);

/* ============================================================
   API Implementation
   ============================================================ */

MyJsonRoot* MyJsonRoot_Create   (MyJsonRoot* root, MyJsonType type) {
    MY_STRUCT_CREATE_RULE(root, MyJsonRoot);
    root->body = MyJson_Create(type, NULL);
    return root;
}
void        MyJsonRoot_Destroy  (MyJsonRoot* root) {
    MY_ASSERT_PTR(root);
    MyJson_Destroy(root->body);
    MY_STRUCT_DESTROY_RULE(root);
}
MyJson*     MyJsonRoot_GetBody     (MyJsonRoot* root) {
    MY_ASSERT_PTR(root);
    return root->body;
}

MyJsonRoot* MyJsonRoot_Parse    (MyJsonRoot* root, const char* source) {
    MY_ASSERT_PTR(source);
    MY_STRUCT_CREATE_RULE(root, MyJsonRoot);
    
    MyJsonWriter writer = {0};
    writer.ln = 1;
    writer.col = 1;
    writer.source = source;
    root->body = MyJson_Parse(&writer);
    root->body->key = MyStrdup("MyJson.root", NULL);
    MyJson_Ignore(&writer);
    MY_JSON_ASSERT((&writer), *writer.source == '\0', "Trailing garbage after JSON value");
    return root;
}
char*       MyJsonRoot_Print    (MyJsonRoot* root, bool pretty) {
    MY_ASSERT_PTR(root);

    MyString str;
    MyString_Create(&str);
    MyJson_Print(root->body, &str, pretty, 0);
    MyString_Append(&str, "\n\n");
    char* dup = MyStrdup(MyString_Cstr(&str), NULL);
    MyString_Destroy(&str);
    return dup;
}

const char* MyJson_Key          (MyJson* json) {
    MY_ASSERT_PTR(json);
    return json->key;
}
MyJsonType  MyJson_Type         (MyJson* json) {
    MY_ASSERT_PTR(json);
    return json->type;
}
bool        MyJson_Bool         (MyJson* json) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_BOOL, "Json element is not a boolean");
    return json->data.boolean;
}
int64_t     MyJson_Integer      (MyJson* json) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_INTEGER, "Json element is not an integer");
    return json->data.integer;
}
double      MyJson_Decimal      (MyJson* json) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_DECIMAL, "Json element is not a decimal");
    return json->data.decimal;
}
double      MyJson_Number       (MyJson* json) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_INTEGER || json->type == MY_JSON_DECIMAL, "Json element is not a number");
    if (json->type == MY_JSON_INTEGER) { return (double)json->data.integer; }
    return json->data.decimal;
}
char*       MyJson_String       (MyJson* json) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_STRING, "Json element is not a string");
    return json->data.string;
}

bool        MyJson_IsNull       (MyJson* json) {
    MY_ASSERT_PTR(json);
    return json->type == MY_JSON_NULL;
}
bool        MyJson_IsBool       (MyJson* json) {
    MY_ASSERT_PTR(json);
    return json->type == MY_JSON_BOOL;
}
bool        MyJson_IsInteger    (MyJson* json) {
    MY_ASSERT_PTR(json);
    return json->type == MY_JSON_INTEGER;
}
bool        MyJson_IsDecimal    (MyJson* json) {
    MY_ASSERT_PTR(json);
    return json->type == MY_JSON_DECIMAL;
}
bool        MyJson_IsNumber     (MyJson* json) {
    MY_ASSERT_PTR(json);
    return json->type == MY_JSON_INTEGER || json->type == MY_JSON_DECIMAL;
}
bool        MyJson_IsString     (MyJson* json) {
    MY_ASSERT_PTR(json);
    return json->type == MY_JSON_STRING;
}
bool        MyJson_IsArray      (MyJson* json) {
    MY_ASSERT_PTR(json);
    return json->type == MY_JSON_ARRAY;
}
bool        MyJson_IsObject     (MyJson* json) {
    MY_ASSERT_PTR(json);
    return json->type == MY_JSON_OBJECT;
}

size_t      MyJson_ArraySize    (MyJson* json) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    return json->data.array.size;
}
void        MyJson_ArrayRemove  (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJsonArray_Erase(&json->data.array, index);
}
void        MyJson_ArrayForEach (MyJson* json, bool(*foreach)(MyJson* it, size_t index, void* userp), void* userp) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    for (size_t i = 0; i < json->data.array.size; i++) {
        MyJson* object = MyJsonArray_Get(&json->data.array, i);
        if (foreach(object, i, userp)) {
            return;
        }
    }
}

size_t      MyJson_ObjectSize   (MyJson* json) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    return json->data.array.size;
}
void        MyJson_ObjectRemove (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    for (size_t i = 0; i < json->data.array.size; i++) {
        MyJson* object = MyJsonArray_Get(&json->data.array, i);
        if (strcmp(object->key, key) == 0) {
            MyJsonArray_Erase(&json->data.array, i);
            return;
        }
    }
}
void        MyJson_ObjectForEach(MyJson* json, bool(*foreach)(MyJson* it, const char* key, void* userp), void* userp) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    for (size_t i = 0; i < json->data.array.size; i++) {
        MyJson* object = MyJsonArray_Get(&json->data.array, i);
        if (foreach(object, object->key, userp)) {
            return;
        }
    }
}

MyJson*     MyJson_KGet             (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    return MyJson_Find(json, key);
}
MyJson*     MyJson_KGetPath         (MyJson* json, const char* path) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(path);

    char key[1024] = {0};
    MyJson* current = json;
    
    while (true) {
        size_t length = 0;

        while (*path && *path != '.' && *path != '[') {
            MY_ASSERT(length + 1 < sizeof(key), "Key too long");
            key[length++] = *path++;
        }
        key[length] = '\0';

        MY_NASSERT(length == 0 && *path != '[', "Empty key but no '[' was founded");

        if (length > 0) {
            MY_ASSERT(current->type == MY_JSON_OBJECT, "Not an object");
            MyJson* child = MyJson_Find(current, key);
            MY_ASSERT(child != NULL, "Key '%s' not found", key);
            current = child;
        }

        if (*path == '[') {
            MY_ASSERT(isdigit(*++path), "Expected digit after '['");

            size_t index = 0;
            while (isdigit(*path)) {
                index = index * 10 + (*path - '0');
                path++;
            }

            MY_ASSERT(*path++ == ']', "Missing ']'");
            MY_ASSERT(current->type == MY_JSON_ARRAY, "Not an array");
            current = MyJson_Get(current, index);
        }

        if (*path == '.') {
            MY_ASSERT(current->type == MY_JSON_OBJECT, "%s is not an object", current->key ? current->key : "<root>");
            path++;
            continue;
        }

        if (*path == '\0') {
            break;
        }

        MY_ASSERT(false, "Unexpected character '%c' in path", *path);
    }

    return current;
}
bool        MyJson_KGetBool         (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MyJson* child = MyJson_Find(json, key);
    MY_ASSERT(child != NULL, "Inexisting element with key = %s", key);
    MY_ASSERT(child->type == MY_JSON_BOOL, "Json element is not a boolean");
    return child->data.boolean;
}
int64_t     MyJson_KGetInteger      (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MyJson* child = MyJson_Find(json, key);
    MY_ASSERT(child != NULL, "Inexisting element with key = %s", key);
    MY_ASSERT(child->type == MY_JSON_INTEGER, "Json element is not an integer");
    return child->data.integer;
}
double      MyJson_KGetDecimal      (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MyJson* child = MyJson_Find(json, key);
    MY_ASSERT(child != NULL, "Inexisting element with key = %s", key);
    MY_ASSERT(child->type == MY_JSON_DECIMAL, "Json element is not a decimal");
    return child->data.decimal;
}
double      MyJson_KGetNumber       (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MyJson* child = MyJson_Find(json, key);
    MY_ASSERT(child != NULL, "Inexisting element with key = %s", key);
    MY_ASSERT(child->type == MY_JSON_INTEGER || child->type == MY_JSON_DECIMAL, "Json element is not a number");
    if (json->type == MY_JSON_INTEGER) { return (double)json->data.integer; }
    return json->data.decimal;
}
char*       MyJson_KGetString       (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MyJson* child = MyJson_Find(json, key);
    MY_ASSERT(child != NULL, "Inexisting element with key = %s", key);
    MY_ASSERT(child->type == MY_JSON_STRING, "Json element is not a string");
    return child->data.string;
}
MyJson*     MyJson_KGetArray        (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MyJson* child = MyJson_Find(json, key);
    MY_ASSERT(child != NULL, "Inexisting element with key = %s", key);
    MY_ASSERT(child->type == MY_JSON_ARRAY, "Json element is not an array");
    return child;
}
MyJson*     MyJson_KGetObject       (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MyJson* child = MyJson_Find(json, key);
    MY_ASSERT(child != NULL, "Inexisting element with key = %s", key);
    MY_ASSERT(child->type == MY_JSON_OBJECT, "Json element is not an object");
    return child;
}

void        MyJson_KSetNull      (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MyJson* child = MyJson_Find(json, key);
    if (child) { MyJson_NullReplace(child); }
    else {
        child = MyJson_Add(json, MY_JSON_NULL, key);
    }
}
void        MyJson_KSetBool      (MyJson* json, const char* key, bool boolean) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MyJson* child = MyJson_Find(json, key);
    if (child) { MyJson_BoolReplace(child, boolean); }
    else {
        child = MyJson_Add(json, MY_JSON_BOOL, key);
        child->data.decimal = boolean;
    }
}
void        MyJson_KSetInteger   (MyJson* json, const char* key, int64_t integer) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MyJson* child = MyJson_Find(json, key);
    if (child) { MyJson_IntegerReplace(child, integer); }
    else {
        child = MyJson_Add(json, MY_JSON_INTEGER, key);
        child->data.integer = integer;
    }
}
void        MyJson_KSetDecimal   (MyJson* json, const char* key, double decimal) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MyJson* child = MyJson_Find(json, key);
    if (child) { MyJson_DecimalReplace(child, decimal); }
    else {
        child = MyJson_Add(json, MY_JSON_DECIMAL, key);
        child->data.decimal = decimal;
    }
}
void        MyJson_KSetString    (MyJson* json, const char* key, const char* string) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MyJson* child = MyJson_Find(json, key);
    if (child) { MyJson_StringReplace(child, string); }
    else {
        child = MyJson_Add(json, MY_JSON_STRING, key);
        child->data.string = MyStrdup(string, NULL);
    }
}
MyJson*     MyJson_KSetArray     (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MyJson* child = MyJson_Find(json, key);
    if (child) { MyJson_ArrayReplace(child); }
    else {
        child = MyJson_Add(json, MY_JSON_ARRAY, key);
    }
    return child;
}
MyJson*     MyJson_KSetObject    (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MyJson* child = MyJson_Find(json, key);
    if (child) { MyJson_ObjectReplace(child); }
    else {
        child = MyJson_Add(json, MY_JSON_OBJECT, key);
    }
    return child;
}

void        MyJson_KInsertNull      (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MY_ASSERT(MyJson_Find(json, key) == NULL, "Existing element with key = %s", key);
    MyJson_Add(json, MY_JSON_NULL, key);
}
void        MyJson_KInsertBool      (MyJson* json, const char* key, bool boolean) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MY_ASSERT(MyJson_Find(json, key) == NULL, "Existing element with key = %s", key);
    MyJson_Add(json, MY_JSON_BOOL, key)->data.boolean = boolean;
}
void        MyJson_KInsertInteger   (MyJson* json, const char* key, int64_t integer) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MY_ASSERT(MyJson_Find(json, key) == NULL, "Existing element with key = %s", key);
    MyJson_Add(json, MY_JSON_INTEGER, key)->data.integer = integer;
}
void        MyJson_KInsertDecimal   (MyJson* json, const char* key, double decimal) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MY_ASSERT(MyJson_Find(json, key) == NULL, "Existing element with key = %s", key);
    MyJson_Add(json, MY_JSON_DECIMAL, key)->data.decimal = decimal;
}
void        MyJson_KInsertString    (MyJson* json, const char* key, const char* string) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MY_ASSERT(MyJson_Find(json, key) == NULL, "Existing element with key = %s", key);
    MyJson_Add(json, MY_JSON_STRING, key)->data.string = MyStrdup(string, NULL);
}
MyJson*     MyJson_KInsertArray     (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MY_ASSERT(MyJson_Find(json, key) == NULL, "Existing element with key = %s", key);
    return MyJson_Add(json, MY_JSON_ARRAY, key);
}
MyJson*     MyJson_KInsertObject    (MyJson* json, const char* key) {
    MY_ASSERT_PTR(json);
    MY_ASSERT_PTR(key);
    MY_ASSERT(json->type == MY_JSON_OBJECT, "Json element is not an object");
    MY_ASSERT(MyJson_Find(json, key) == NULL, "Existing element with key = %s", key);
    return MyJson_Add(json, MY_JSON_OBJECT, key);
}

MyJson*     MyJson_Get          (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Trying to get an array element in a non array object");
    return MyJsonArray_Get(&json->data.array, index);
}
bool        MyJson_GetBool      (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Trying to get an array element in a non array object");
    MyJson* child = MyJsonArray_Get(&json->data.array, index);
    MY_ASSERT(child->type == MY_JSON_BOOL, "Json element is not boolean");
    return child->data.boolean;
}
int64_t     MyJson_GetInteger   (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Trying to get an array element in a non array object");
    MyJson* child = MyJsonArray_Get(&json->data.array, index);
    MY_ASSERT(child->type == MY_JSON_INTEGER, "Json element is not integer");
    return child->data.integer;
}
double      MyJson_GetDouble    (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Trying to get an array element in a non array object");
    MyJson* child = MyJsonArray_Get(&json->data.array, index);
    MY_ASSERT(child->type == MY_JSON_DECIMAL, "Json element is not decimal");
    return child->data.decimal;
}
double      MyJson_GetNumber    (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Trying to get an array element in a non array object");
    MyJson* child = MyJsonArray_Get(&json->data.array, index);
    MY_ASSERT(child->type == MY_JSON_INTEGER || child->type == MY_JSON_DECIMAL, "Json element is not a number");
    if (json->type == MY_JSON_INTEGER) { return (double)json->data.integer; }
    return json->data.decimal;
}
char*       MyJson_GetString    (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Trying to get an array element in a non array object");
    MyJson* child = MyJsonArray_Get(&json->data.array, index);
    MY_ASSERT(child->type == MY_JSON_STRING, "Json element is not string");
    return child->data.string;
}
MyJson*     MyJson_GetArray     (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Trying to get an array element in a non array object");
    MyJson* child = MyJsonArray_Get(&json->data.array, index);
    MY_ASSERT(child->type == MY_JSON_ARRAY, "Json element is not array");
    return child;
}
MyJson*     MyJson_GetObject    (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Trying to get an array element in a non array object");
    MyJson* child = MyJsonArray_Get(&json->data.array, index);
    MY_ASSERT(child->type == MY_JSON_OBJECT, "Json element is not object");
    return child;
}

void        MyJson_SetNull      (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Get(json, index);
    MyJson_NullReplace(child);
}
void        MyJson_SetBool      (MyJson* json, size_t index, bool boolean) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Get(json, index);
    MyJson_BoolReplace(child, boolean);
}
void        MyJson_SetInteger   (MyJson* json, size_t index, int64_t integer) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Get(json, index);
    MyJson_IntegerReplace(child, integer);
}
void        MyJson_SetDecimal   (MyJson* json, size_t index, double decimal) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Get(json, index);
    MyJson_DecimalReplace(child, decimal);
}
void        MyJson_SetString    (MyJson* json, size_t index, const char* string) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Get(json, index);
    MyJson_StringReplace(child, string);
}
MyJson*     MyJson_SetArray     (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Get(json, index);
    MyJson_ArrayReplace(child);
    return child;
}
MyJson*     MyJson_SetObject    (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Get(json, index);
    MyJson_ObjectReplace(child);
    return child;
}

void        MyJson_InsertNull   (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Create(MY_JSON_NULL, NULL);
    MyJsonArray_Insert(&json->data.array, index, child);
}
void        MyJson_InsertBool   (MyJson* json, size_t index, bool boolean) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Create(MY_JSON_BOOL, NULL);
    child->data.boolean = boolean;
    MyJsonArray_Insert(&json->data.array, index, child);
}
void        MyJson_InsertInteger(MyJson* json, size_t index, int64_t integer) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Create(MY_JSON_INTEGER, NULL);
    child->data.integer = integer;
    MyJsonArray_Insert(&json->data.array, index, child);
}
void        MyJson_InsertDecimal(MyJson* json, size_t index, double decimal) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Create(MY_JSON_DECIMAL, NULL);
    child->data.decimal = decimal;
    MyJsonArray_Insert(&json->data.array, index, child);
}
void        MyJson_InsertString (MyJson* json, size_t index, const char* string) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Create(MY_JSON_STRING, NULL);
    child->data.string = MyStrdup(string, NULL);
    MyJsonArray_Insert(&json->data.array, index, child);
}
MyJson*     MyJson_InsertArray  (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Create(MY_JSON_ARRAY, NULL);
    MyJsonArray_Insert(&json->data.array, index, child);
    return child;
}
MyJson*     MyJson_InsertObject (MyJson* json, size_t index) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Create(MY_JSON_OBJECT, NULL);
    MyJsonArray_Insert(&json->data.array, index, child);
    return child;
}

void        MyJson_PushNull     (MyJson* json) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson_Add(json, MY_JSON_NULL, NULL);
}
void        MyJson_PushBool     (MyJson* json, bool boolean) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Add(json, MY_JSON_BOOL, NULL);
    child->data.boolean = boolean;
}
void        MyJson_PushInteger  (MyJson* json, int64_t integer) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Add(json, MY_JSON_INTEGER, NULL);
    child->data.integer = integer;
}
void        MyJson_PushDecimal  (MyJson* json, double decimal) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Add(json, MY_JSON_DECIMAL, NULL);
    child->data.decimal = decimal;
}
void        MyJson_PushString   (MyJson* json, const char* string) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    MyJson* child = MyJson_Add(json, MY_JSON_STRING, NULL);
    child->data.string = MyStrdup(string, NULL);
}
MyJson*     MyJson_PushArray    (MyJson* json) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    return MyJson_Add(json, MY_JSON_ARRAY, NULL);
}
MyJson*     MyJson_PushObject   (MyJson* json) {
    MY_ASSERT_PTR(json);
    MY_ASSERT(json->type == MY_JSON_ARRAY, "Json element is not an array");
    return MyJson_Add(json, MY_JSON_OBJECT, NULL);
}

/* ============================================================
   Helpers Implementation
   ============================================================ */

static int MyJson_HexVal(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    return -1;
}
static int MyJson_ParseU4(const char *s) {
    int v = 0;
    for (int i = 0; i < 4; i++) {
        int h = MyJson_HexVal(s[i]);
        if (h < 0) { return -1; }
        v = (v << 4) | h;
    }
    return v;
}
static int MyJson_UTF8Encode(char *out, int cp) {
    if (cp <= 0x7F) {
        out[0] = (char)cp;
        return 1;
    } else if (cp <= 0x7FF) {
        out[0] = 0xC0 | (cp >> 6);
        out[1] = 0x80 | (cp & 0x3F);
        return 2;
    } else if (cp <= 0xFFFF) {
        out[0] = 0xE0 | (cp >> 12);
        out[1] = 0x80 | ((cp >> 6) & 0x3F);
        out[2] = 0x80 | (cp & 0x3F);
        return 3;
    } else {
        out[0] = 0xF0 | (cp >> 18);
        out[1] = 0x80 | ((cp >> 12) & 0x3F);
        out[2] = 0x80 | ((cp >> 6) & 0x3F);
        out[3] = 0x80 | (cp & 0x3F);
        return 4;
    }
}

static void MyJsonString_CtoJson(MyString* str, const char* src) {
    while (*src) {
        if (*src == '\"') { MyString_Append(str, "\\\""); }
        else if (*src == '\\') { MyString_Append(str, "\\\\"); }
        else if (*src == '\n') { MyString_Append(str, "\\n"); }
        else if (*src == '\t') { MyString_Append(str, "\\t"); }
        else if (*src == '\r') { MyString_Append(str, "\\r"); }
        else if (*src == '\b') { MyString_Append(str, "\\b"); }
        else if (*src == '\f') { MyString_Append(str, "\\f"); }
        else if ((unsigned char)*src < 0x20) {
            static const char hex[] = "0123456789ABCDEF";
            unsigned char c = (unsigned char)*src;
            char buf[6];
            buf[0] = '\\';
            buf[1] = 'u';
            buf[2] = '0';
            buf[3] = '0';
            buf[4] = hex[(c >> 4) & 0xF];
            buf[5] = hex[c & 0xF];

            MyString_AppendN(str, buf, 6);
        }
        else { MyString_PushBack(str, *src); }
        src++;
    }
}
static void MyJsonString_JsontoC(char* src, size_t count) {   
    size_t cursor = 0;
    for (size_t i = 0; i < count; i++) {
        if (src[i] != '\\') {
            src[cursor++] = src[i];
            continue;
        }

        i++;
        MY_ASSERT(i < count, "Invalid escape at end of string");
        char c = src[i];

        if (c == '\"')      { src[cursor++] = '\"'; continue; }
        else if (c == '\\') { src[cursor++] = '\\'; continue; }
        else if (c == '/')  { src[cursor++] =  '/'; continue; }
        else if (c == 'n')  { src[cursor++] = '\n'; continue; }
        else if (c == 't')  { src[cursor++] = '\t'; continue; }
        else if (c == 'r')  { src[cursor++] = '\r'; continue; }
        else if (c == 'b')  { src[cursor++] = '\b'; continue; }
        else if (c == 'f')  { src[cursor++] = '\f'; continue; }
        else if (c != 'u')  { MyLog(MY_FATAL, "Invalid escape sequence '%c'", c); }

        // Need 4 hex digits
        MY_ASSERT(i + 4 < count, "Invalid \\u escape");
        
        int cp = MyJson_ParseU4(&src[i + 1]);
        MY_ASSERT(cp >= 0, "Invalid hex in \\u escape");
        MY_NASSERT(cp >= 0xDC00 && cp <= 0xDFFF, "Unexpected low surrogate");
        
        i += 4;

        // Handle surrogate pairs
        if (cp >= 0xD800 && cp <= 0xDBFF) {
            // high surrogate, expect \uXXXX next
            MY_ASSERT(i + 6 < count, "Invalid surrogate pair length");
            MY_ASSERT(src[i + 1] == '\\' && src[i + 2] == 'u', "Invalid surrogate pair");

            int low = MyJson_ParseU4(&src[i + 3]);
            MY_ASSERT(low >= 0xDC00 && low <= 0xDFFF, "Invalid low surrogate");

            // Combine into full codepoint
            cp = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
            i += 6; // skip \uXXXX
        }

        char utf8[4];
        int len = MyJson_UTF8Encode(utf8, cp);

        for (int k = 0; k < len; k++) {
            src[cursor++] = utf8[k];
        }
    }
    src[cursor] = '\0';
}

static MyJson* MyJson_Create(MyJsonType type, const char* key) {
    MyJson* json = NULL;
    MY_JSON_ALLOC(json);
    json->type = type;
    if (key) { json->key = MyStrdup(key, NULL); }
    if (type == MY_JSON_ARRAY || type == MY_JSON_OBJECT) { MyJsonArray_Create(&json->data.array); }
    return json;
}
static void MyJson_Destroy(MyJson* json) {
    MY_FREE_IF(json->key);

    if (json->type == MY_JSON_STRING) {
        MY_FREE(json->data.string);
    } else if (json->type == MY_JSON_ARRAY || json->type == MY_JSON_OBJECT) {
        for (size_t i = 0; i < json->data.array.size; i++) {
            MyJson_Destroy(MyJsonArray_Get(&json->data.array, i));
        }
        MyJsonArray_Clear(&json->data.array);
        MyJsonArray_Destroy(&json->data.array);
    }

    MY_JSON_FREE(json);
}
static void MyJson_Erase(MyJson* json) {
    if (json->type == MY_JSON_STRING) {
        MY_FREE(json->data.string);
    } else if (json->type == MY_JSON_ARRAY || json->type == MY_JSON_OBJECT) {
        for (size_t i = 0; i < json->data.array.size; i++) {
            MyJson_Destroy(MyJsonArray_Get(&json->data.array, i));
        }
        MyJsonArray_Clear(&json->data.array);
        MyJsonArray_Destroy(&json->data.array);
    }
}
static MyJson* MyJson_Add(MyJson* parent, MyJsonType type, const char* key) {
    MyJson* child = MyJson_Create(type, key);
    MyJsonArray_PushBack(&parent->data.array, child);
    return child;
}
static MyJson* MyJson_Find(MyJson* object, const char* key) {
    for (size_t i = 0; i < object->data.array.size; i++) {
        MyJson* child = MyJsonArray_Get(&object->data.array, i);
        if (strcmp(child->key, key) == 0) {
            return child;
        }
    }
    return NULL;
}

static void MyJson_NullReplace(MyJson* json) {
    MyJson_Erase(json);
}
static void MyJson_BoolReplace(MyJson* json, bool boolean) {
    MyJson_Erase(json);
    json->data.boolean = boolean;
}
static void MyJson_IntegerReplace(MyJson* json, int64_t integer) {
    MyJson_Erase(json);
    json->data.integer = integer;
}
static void MyJson_DecimalReplace(MyJson* json, double decimal) {
    MyJson_Erase(json);
    json->data.decimal = decimal;
}
static void MyJson_StringReplace(MyJson* json, const char* string) {
    MyJson_Erase(json);
    json->data.string = MyStrdup(string, NULL);
}
static void MyJson_ArrayReplace(MyJson* json) {
    MyJson_Erase(json);
    MyJsonArray_Create(&json->data.array);
}
static void MyJson_ObjectReplace(MyJson* json) {
    MyJson_Erase(json);
    MyJsonArray_Create(&json->data.array);
}

#define MY_JSON_INDENT(str, pretty, indent) do { if (pretty) { MyString_AppendCh(str, '\t', indent); } } while(0)
#define MY_JSON_NEW_LINE(str, pretty, indent) do { if (pretty) { MyString_PushBack(str, '\n'); } } while(0)

static void MyJson_Print(MyJson* json, MyString* str, bool pretty, size_t indent) {
    if (json->type == MY_JSON_NULL) {
        MyString_Append(str, "null");
        return;
    }
    
    if (json->type == MY_JSON_BOOL) {
        MyString_Append(str, MY_TERNARY(json->data.boolean, "true", "false"));
        return;
    }

    if (json->type == MY_JSON_INTEGER) {
        const char* tos = MyI64tos(json->data.integer, false, false);
        MyString_Append(str, tos);
        return;
    }

    if (json->type == MY_JSON_DECIMAL) {
        const char* tos = MyF64tos(json->data.decimal, 15, false, false);
        size_t length = strlen(tos);
        for (size_t i = length - 1; i > 0 && tos[i] == '0'; i--) {
            length--;
        }
        if (tos[length - 1] == '.') { length--; }

        MyString_AppendN(str, tos, length);
        return;
    }

    if (json->type == MY_JSON_STRING) {
        MyString_PushBack(str, '\"');
        MyJsonString_CtoJson(str, json->data.string);
        MyString_PushBack(str, '\"');
        return;
    }

    if (json->type == MY_JSON_ARRAY) {
        MyJson_PrintArray(json, str, pretty, indent);
        return;
    }

    if (json->type == MY_JSON_OBJECT) {
        MyJson_PrintObject(json, str, pretty, indent);
        return;
    }
}
static void MyJson_PrintKey(MyJson* json, MyString* str) {
    MyString_PushBack(str, '\"');
    MyString_Append(str, json->key);
    MyString_Append(str, "\": ");
}
static void MyJson_PrintArray(MyJson* json, MyString* str, bool pretty, size_t indent) {
    MyString_PushBack(str, '[');
    MY_JSON_NEW_LINE(str, pretty, indent);

    MyJsonArray* array = &json->data.array;
    size_t size = array->size;
    if (size == 0) {
        MY_JSON_INDENT(str, pretty, indent);
        MyString_PushBack(str, ']');
        return;
    }

    for (size_t i = 0; i < size - 1; i++) {
        MyJson* element = MyJsonArray_Get(array, i);
        MY_JSON_INDENT(str, pretty, indent + 1);
        MyJson_Print(element, str, pretty, indent + 1);
        MyString_Append(str, ", ");
        MY_JSON_NEW_LINE(str, pretty, indent);
    }
    
    MyJson* element = MyJsonArray_Get(array, size - 1);
    MY_JSON_INDENT(str, pretty, indent + 1);
    MyJson_Print(element, str, pretty, indent + 1);
    MY_JSON_NEW_LINE(str, pretty, indent);

    MY_JSON_INDENT(str, pretty, indent);
    MyString_PushBack(str, ']');
}
static void MyJson_PrintObject(MyJson* json, MyString* str, bool pretty, size_t indent) {
    MyString_PushBack(str, '{');
    MY_JSON_NEW_LINE(str, pretty, indent);

    MyJsonArray* array = &json->data.array;
    size_t size = array->size;
    if (size == 0) {
        MY_JSON_INDENT(str, pretty, indent);
        MyString_PushBack(str, '}');
        return;
    }

    for (size_t i = 0; i < size - 1; i++) {
        MyJson* element = MyJsonArray_Get(array, i);
        MY_JSON_INDENT(str, pretty, indent + 1);
        MyJson_PrintKey(element, str);
        MyJson_Print(element, str, pretty, indent + 1);
        MyString_Append(str, ", ");
        MY_JSON_NEW_LINE(str, pretty, indent);
    }
    
    MyJson* element = MyJsonArray_Get(array, size - 1);
    MY_JSON_INDENT(str, pretty, indent + 1);
    MyJson_PrintKey(element, str);
    MyJson_Print(element, str, pretty, indent + 1);
    MY_JSON_NEW_LINE(str, pretty, indent);

    MY_JSON_INDENT(str, pretty, indent);
    MyString_PushBack(str, '}');
}

static void MyJson_Ignore(MyJsonWriter* writer) {
    while (true) {
        char c = *writer->source;

        if (c == ' ' || c == '\t') {
            writer->col++;
        } else if (c == '\n') {
            writer->ln++;
            writer->col = 1;
        } else {
            break;
        }

        writer->source++;
    }
}
static char* MyJson_ParseLiteral(MyJsonWriter* writer) {
    MY_JSON_ASSERT(writer, *writer->source == '\"', "String literal '\"' open not founded");
    writer->col++;
    writer->source++;

    size_t length = 0;
    const char* start = writer->source;
    while(*writer->source && *writer->source != '\"') {
        if (*writer->source == '\\') {
            writer->source += 2;
            writer->col += 2;
            length += 2;
            continue;
        }

        MY_JSON_ASSERT(writer, *writer->source != '\b', "Backspace is not valid inside a String literal");
        MY_JSON_ASSERT(writer, *writer->source != '\f', "Form feed is not valid inside a String literal");
        MY_JSON_ASSERT(writer, *writer->source != '\n', "Line feed is not valid inside a String literal");
        MY_JSON_ASSERT(writer, *writer->source != '\r', "Carriage return is not valid inside a String literal");
        MY_JSON_ASSERT(writer, *writer->source != '\t', "Tab is not valid inside a String literal");
        length++;
        writer->col++;
        writer->source++;
    }

    MY_JSON_ASSERT(writer, *writer->source == '\"', "String literal '\"' close not founded");
    writer->col++;
    writer->source++;

    char* literal = NULL;
    MY_MALLOC(literal, char, length + 1);
    memcpy(literal, start, length);
    literal[length] = '\0';
    MyJsonString_JsontoC(literal, length);
    return literal;
}

static MyJson* MyJson_Parse(MyJsonWriter* writer) {
    MyJson* json = NULL;
    MyJson_Ignore(writer);
    if (*writer->source == '{') {
        json = MyJson_ParseObject(writer);
    } else if (*writer->source == '[') {
        json = MyJson_ParseArray(writer);
    } else if (*writer->source == '\"') {
        json = MyJson_ParseString(writer);
    } else if (isdigit(*writer->source) || *writer->source == '-') {
        json = MyJson_ParseIntegerDecimal(writer);
    } else if (strncmp(writer->source, "null", 4) == 0) {
        json = MyJson_ParseNull(writer);
    } else if (strncmp(writer->source, "true", 4) == 0 || strncmp(writer->source, "false", 5) == 0) {
        json = MyJson_ParseBool(writer);
    } else {
        MY_JSON_ASSERT(writer, false, "Invalid token or trailling comma");
    }

    return json;
}
static MyJson* MyJson_ParseNull(MyJsonWriter* writer) {
    MY_JSON_ASSERT(writer, strncmp(writer->source, "null", 4) == 0, "Failed to parse null");
    writer->source += 4;
    return MyJson_Create(MY_JSON_NULL, NULL);
}
static MyJson* MyJson_ParseBool(MyJsonWriter* writer) {
    MyJson* body = MyJson_Create(MY_JSON_BOOL, NULL);
    if (strncmp(writer->source, "true", 4) == 0) {
        body->data.boolean = true;
        writer->source += 4;
    } else if (strncmp(writer->source, "false", 5) == 0)  {
        body->data.boolean = false;
        writer->source += 5;
    } else {
        MY_JSON_ASSERT(writer, *writer->source == '\0', "Failed to parse boolean");
    }
    return body;
}
static MyJson* MyJson_ParseIntegerDecimal(MyJsonWriter* writer) {
    bool negative = false;
    if (*writer->source == '-') {
        negative = true;
        writer->col++;
        writer->source++;
    }

    MyJson* json = NULL;
    MY_JSON_ASSERT(writer, isdigit(*writer->source), "Failed to parse integer/decimal");
    if (*writer->source == '0' && isdigit(writer->source[1])) {
        MY_JSON_ASSERT(writer, false, "Invalid leading zeros");
    }

    const char* temp = writer->source;
    while (isdigit(*temp)) { temp++; }
    bool nonInteger = *temp == '.' || *temp == 'e' || *temp == 'E';

    if (!nonInteger) {
        int64_t integer = 0;
        while (isdigit(*writer->source)) { 
            int digit = *writer->source - '0';
            writer->source++;
            writer->col++;
            if (negative) {
                if (integer < (INT64_MIN + digit) / 10) { goto parseDouble; }
                integer = integer * 10 - digit;
            } else {
                if (integer > (INT64_MAX - digit) / 10) { goto parseDouble; }
                integer = integer * 10 + digit;
            }
        }
        json = MyJson_Create(MY_JSON_INTEGER, NULL);
        json->data.integer = integer;
        return json;
    }

parseDouble:
    char* end;
    double decimal = strtod(writer->source, &end);
    MY_JSON_ASSERT(writer, isfinite(decimal), "Invalid JSON number");
    MY_JSON_ASSERT(writer, end != writer->source, "Invalid strtod number");

    writer->col += (end - writer->source);
    writer->source = end;
    json = MyJson_Create(MY_JSON_DECIMAL, NULL);
    json->data.decimal = decimal;
    return json;
}
static MyJson* MyJson_ParseString(MyJsonWriter* writer) {
    MyJson* json = MyJson_Create(MY_JSON_STRING, NULL);
    json->data.string = MyJson_ParseLiteral(writer);
    return json;
}
static MyJson* MyJson_ParseArray(MyJsonWriter* writer) {
    MY_JSON_ASSERT(writer, *writer->source == '[', "Array '[' open not founded");
    writer->col++;
    writer->source++;
    MyJson_Ignore(writer);
    MyJson* json = MyJson_Create(MY_JSON_ARRAY, NULL);

    if (*writer->source == ']') {
        writer->col++;
        writer->source++;
        return json;
    }

    while (true) {
        MyJson* child = MyJson_Parse(writer);
        MyJsonArray_PushBack(&json->data.array, child);
        MyJson_Ignore(writer);

        if (*writer->source == ',') {
            writer->col++;
            writer->source++;
            MyJson_Ignore(writer);
            continue;
        }
        break;
    };

    MyJson_Ignore(writer);
    MY_JSON_ASSERT(writer, *writer->source == ']', "Array ']' close not founded");
    writer->col++;
    writer->source++;
    return json;
}
static MyJson* MyJson_ParseObject(MyJsonWriter* writer) {
    MY_JSON_ASSERT(writer, *writer->source == '{', "Object '{' open not founded");
    writer->col++;
    writer->source++;
    MyJson_Ignore(writer);
    MyJson* json = MyJson_Create(MY_JSON_OBJECT, NULL);

    if (*writer->source == '}') {
        writer->col++;
        writer->source++;
        return json;
    }

    while (true) {
        char* key = MyJson_ParseLiteral(writer);
        MyJson_Ignore(writer);
        MY_JSON_ASSERT(writer, *writer->source == ':', "':' not founded after key");
        writer->col++;
        writer->source++;
        MyJson_Ignore(writer);

        MyJson* child = MyJson_Parse(writer);
        child->key = key;
        MyJsonArray_PushBack(&json->data.array, child);
        MyJson_Ignore(writer);

        if (*writer->source == ',') {
            writer->col++;
            writer->source++;
            MyJson_Ignore(writer);
            continue;
        }
        break;
    };

    MyJson_Ignore(writer);
    MY_JSON_ASSERT(writer, *writer->source == '}', "Object '}' close not founded");
    writer->col++;
    writer->source++;
    return json;
}

#ifdef __cplusplus
}
#endif