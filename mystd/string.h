#ifndef __MYSTD_STRING_H__
#define __MYSTD_STRING_H__

#include "stddef.h"
#include <mystd/stdio.h>

#ifndef MY_STRING_RESIZE_POLICIE
    #define MY_STRING_RESIZE_POLICIE(size) (size * 2)
#endif /* MY_STRING_RESIZE_POLICIE */

#ifndef MY_STRING_SHRINK_POLICIE
    #define MY_STRING_SHRINK_POLICIE(capacity) (capacity / 4)
#endif /* MY_STRING_SHRINK_POLICIE */

#ifndef MY_STRING_INITIAL_SIZE
    #define MY_STRING_INITIAL_SIZE 10
#endif /* MY_STRING_INITIAL_SIZE */

#ifdef __cplusplus
extern "C" {
#endif

struct MyString;
typedef struct MyString MyString;

MY_RWLOCK_DECLARES(MyString, str, MyString)

MyString*   MyString_Create     (MyString* str);
void        MyString_Destroy    (MyString* str);

char*       MyString_Cstr       (MyString* str);
size_t      MyString_Size       (MyString* str);

void        MyString_Set        (MyString* str, size_t idx, char c);
char        MyString_Get        (MyString* str, size_t idx);
void        MyString_Resize     (MyString* str, size_t size);
void        MyString_Shrink     (MyString* str);

void        MyString_Clear      (MyString* str);
void        MyString_Erase      (MyString* str, size_t idx);
void        MyString_PopBack    (MyString* str);
void        MyString_PopFront   (MyString* str);

void        MyString_Insert     (MyString* str, size_t idx, char c);
void        MyString_PushBack   (MyString* str, char c);
void        MyString_PushFront  (MyString* str, char c);

void        MyString_Memcpy     (MyString* str, size_t idx, const void* src, size_t count);

struct MyString {
    MyStructHeader  header;

    char*           data;
    size_t          size;
    size_t          capacity;
};

#ifdef __cplusplus
}
#endif

#endif /* __MYSTD_STRING_H__ */