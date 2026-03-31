#ifndef __MYSTD_JSON_H__
#define __MYSTD_JSON_H__

#include <mystd/stdlib.h>
#include <mystd/string.h>

typedef struct MyJson MyJson;

typedef enum {
    MY_JSON_NULL,
    MY_JSON_BOOL,
    MY_JSON_INTEGER,
    MY_JSON_DECIMAL,
    MY_JSON_STRING,
    MY_JSON_ARRAY,
    MY_JSON_OBJECT,
} MyJsonType;

#ifdef MY_JSON_VECTOR
#define MY_VECTOR_IMPLEMENTATION
#endif
#define MY_VECTOR_NAME          MyJsonArray
#define MY_VECTOR_DATA_TYPE     MyJson*
#include <mystd/vector.h>

struct MyJson {
    MyJsonType type;
    char* key;

    union {
        bool            boolean;
        int64_t         integer;
        double          decimal;
        char*           string;
        MyJsonArray     array;
    } data;
};

typedef struct {
    MyStructHeader header;
    MyJson*        body;
} MyJsonRoot;

#ifndef MY_JSON_ALLOC
    #define MY_JSON_ALLOC(v) MY_CALLOC(v, MyJson, 1)
#endif
#ifndef MY_JSON_FREE
    #define MY_JSON_FREE(ptr) MY_FREE(ptr)
#endif

MY_RWLOCK_DECLARES(MyJson, json, MyJson)

MyJsonRoot* MyJsonRoot_Create       (MyJsonRoot* root, MyJsonType type);
void        MyJsonRoot_Destroy      (MyJsonRoot* root);
MyJson*     MyJsonRoot_GetBody      (MyJsonRoot* root);

MyJsonRoot* MyJsonRoot_Parse        (MyJsonRoot* root, const char* source);
char*       MyJsonRoot_Print        (MyJsonRoot* root, bool pretty);

const char* MyJson_Key              (MyJson* json);
MyJsonType  MyJson_Type             (MyJson* json);
bool        MyJson_IsNull           (MyJson* json);
bool        MyJson_IsBool           (MyJson* json);
bool        MyJson_IsInteger        (MyJson* json);
bool        MyJson_IsDecimal        (MyJson* json);
bool        MyJson_IsString         (MyJson* json);
bool        MyJson_IsArray          (MyJson* json);
bool        MyJson_IsObject         (MyJson* json);

size_t      MyJson_ArraySize        (MyJson* json);
void        MyJson_ArrayRemove      (MyJson* json, size_t index);
void        MyJson_ArrayForEach     (MyJson* json, bool(*foreach)(MyJson* it, size_t index, void* userp), void* userp);

size_t      MyJson_ObjectSize       (MyJson* json);
void        MyJson_ObjectRemove     (MyJson* json, const char* key);
void        MyJson_ObjectForEach    (MyJson* json, bool(*foreach)(MyJson* it, const char* key, void* userp), void* userp);

MyJson*     MyJson_KGet             (MyJson* json, const char* key);
MyJson*     MyJson_KGetPath         (MyJson* json, const char* path);
bool        MyJson_KGetBool         (MyJson* json, const char* key);
int64_t     MyJson_KGetInteger      (MyJson* json, const char* key);
double      MyJson_KGetDecimal      (MyJson* json, const char* key);
char*       MyJson_KGetString       (MyJson* json, const char* key);
MyJson*     MyJson_KGetArray        (MyJson* json, const char* key);
MyJson*     MyJson_KGetObject       (MyJson* json, const char* key);

void        MyJson_KSetNull         (MyJson* json, const char* key);
void        MyJson_KSetBool         (MyJson* json, const char* key, bool boolean);
void        MyJson_KSetInteger      (MyJson* json, const char* key, int64_t integer);
void        MyJson_KSetDecimal      (MyJson* json, const char* key, double decimal);
void        MyJson_KSetString       (MyJson* json, const char* key, const char* string);
MyJson*     MyJson_KSetArray        (MyJson* json, const char* key);
MyJson*     MyJson_KSetObject       (MyJson* json, const char* key);

void        MyJson_KInsertNull      (MyJson* json, const char* key);
void        MyJson_KInsertBool      (MyJson* json, const char* key, bool boolean);
void        MyJson_KInsertInteger   (MyJson* json, const char* key, int64_t integer);
void        MyJson_KInsertDecimal   (MyJson* json, const char* key, double decimal);
void        MyJson_KInsertString    (MyJson* json, const char* key, const char* string);
MyJson*     MyJson_KInsertArray     (MyJson* json, const char* key);
MyJson*     MyJson_KInsertObject    (MyJson* json, const char* key);

MyJson*     MyJson_Get              (MyJson* json, size_t index);
bool        MyJson_GetBool          (MyJson* json, size_t index);
int64_t     MyJson_GetInteger       (MyJson* json, size_t index);
double      MyJson_GetDecimal       (MyJson* json, size_t index);
char*       MyJson_GetString        (MyJson* json, size_t index);
MyJson*     MyJson_GetArray         (MyJson* json, size_t index);
MyJson*     MyJson_GetObject        (MyJson* json, size_t index);

void        MyJson_SetNull          (MyJson* json, size_t index);
void        MyJson_SetBool          (MyJson* json, size_t index, bool boolean);
void        MyJson_SetInteger       (MyJson* json, size_t index, int64_t integer);
void        MyJson_SetDecimal       (MyJson* json, size_t index, double decimal);
void        MyJson_SetString        (MyJson* json, size_t index, const char* string);
MyJson*     MyJson_SetArray         (MyJson* json, size_t index);
MyJson*     MyJson_SetObject        (MyJson* json, size_t index);

void        MyJson_InsertNull       (MyJson* json, size_t index);
void        MyJson_InsertBool       (MyJson* json, size_t index, bool boolean);
void        MyJson_InsertInteger    (MyJson* json, size_t index, int64_t integer);
void        MyJson_InsertDecimal    (MyJson* json, size_t index, double decimal);
void        MyJson_InsertString     (MyJson* json, size_t index, const char* string);
MyJson*     MyJson_InsertArray      (MyJson* json, size_t index);
MyJson*     MyJson_InsertObject     (MyJson* json, size_t index);

void        MyJson_PushNull         (MyJson* json);
void        MyJson_PushBool         (MyJson* json, bool boolean);
void        MyJson_PushInteger      (MyJson* json, int64_t integer);
void        MyJson_PushDecimal      (MyJson* json, double decimal);
void        MyJson_PushString       (MyJson* json, const char* string);
MyJson*     MyJson_PushArray        (MyJson* json);
MyJson*     MyJson_PushObject       (MyJson* json);

#endif /* __MYSTD_JSON_H__ */