#ifndef __MYSTD_STRING_H__
#define __MYSTD_STRING_H__

#include <mystd/stdlib.h>

#ifndef MY_STRING_RESIZE_POLICIE
    #define MY_STRING_RESIZE_POLICIE(size) (size * 2)
#endif

#ifndef MY_STRING_SHRINK_POLICIE
    #define MY_STRING_SHRINK_POLICIE(capacity) (capacity / 4)
#endif

#ifndef MY_STRING_INITIAL_SIZE
    #define MY_STRING_INITIAL_SIZE 256
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    MyStructHeader  header;

    char*           data;
    size_t          size;
    size_t          capacity;
} MyString;

MY_RWLOCK_DECLARES(MyString, str, MyString)

MyString*   MyString_Create     (MyString* str);
void        MyString_Destroy    (MyString* str);

char*       MyString_Cstr       (MyString* str);
size_t      MyString_Size       (MyString* str);

void        MyString_Set        (MyString* str, size_t index, char ch);
char        MyString_Get        (MyString* str, size_t index);
void        MyString_Resize     (MyString* str, size_t size);
void        MyString_Reserve    (MyString* str, size_t capacity);
void        MyString_Shrink     (MyString* str);

void        MyString_Clear      (MyString* str);
void        MyString_Erase      (MyString* str, size_t index);
void        MyString_PopBack    (MyString* str);
void        MyString_PopFront   (MyString* str);

void        MyString_Insert     (MyString* str, size_t index, char ch);
void        MyString_PushBack   (MyString* str, char ch);
void        MyString_PushFront  (MyString* str, char ch);

void        MyString_Memset     (MyString* str, char ch, size_t start, size_t count);
void        MyString_Memcpy     (MyString* str, const void* src, size_t start, size_t count);

void        MyString_AppendCh   (MyString* str, char ch, size_t count);
void        MyString_AppendN    (MyString* str, const char* src, size_t count);
void        MyString_Append     (MyString* str, const char* src);

#ifdef __cplusplus
}
#endif

#endif /* __MYSTD_STRING_H__ */