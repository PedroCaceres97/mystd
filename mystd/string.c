#include "mystd/stdlib.h"
#include <mystd/string.h>

#ifdef __cplusplus
extern "C" {
#endif

MY_RWLOCK_DEFINES(MyString, str, MyString)

MyString*   MyString_Create     (MyString* str) {
    MY_STRUCT_CREATE_RULE(str, MyString);

    str->size = 0;
    str->capacity = MY_STRING_INITIAL_SIZE;
    MY_CALLOC(str->data, char, str->capacity + 1);

    return str;
}
void        MyString_Destroy    (MyString* str) {
    MY_ASSERT_PTR(str);

    MY_FREE(str->data);
    MY_STRUCT_DESTROY_RULE(str);
}

char*       MyString_Cstr       (MyString* str) {
    MY_ASSERT_PTR(str);
    return str->data;
}
size_t      MyString_Size       (MyString* str) {
    MY_ASSERT_PTR(str);
    return str->size;
}

void        MyString_Set        (MyString* str, size_t index, char ch) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(index, str->size);

    str->data[index] = ch;
}
char        MyString_Get        (MyString* str, size_t index) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(index, str->size);

    return str->data[index];
}
void        MyString_Resize     (MyString* str, size_t size) {
    MY_ASSERT_PTR(str);

    if (size == str->size) {
        return;
    }

    if (size == 0) {
        MyString_Clear(str);
        return;
    }

    if (size > str->capacity) {
        str->capacity = MY_TERNARY(
            size < MY_STRING_INITIAL_SIZE,
            MY_STRING_INITIAL_SIZE,
            MY_STRING_RESIZE_POLICIE(size)
        );
        MY_REALLOC(str->data, char, str->data, str->capacity + 1);
        memset(&str->data[str->size], 0, (str->capacity - str->size) + 1);
    }

    str->size = size;
    str->data[str->size] = '\0';
}
void        MyString_Reserve    (MyString* str, size_t capacity) {
    MY_ASSERT_PTR(str);

    if (capacity <= str->capacity) {
        return;
    }

    if (capacity == 0) {
        MyString_Clear(str);
        return;
    }

    str->capacity = capacity;
    MY_REALLOC(str->data, char, str->data, str->capacity + 1);
    memset(&str->data[str->size], 0, (str->capacity - str->size) + 1);
}
void        MyString_Shrink     (MyString* str) {
    MY_ASSERT_PTR(str);

    if (str->size < MY_STRING_SHRINK_POLICIE(str->capacity) && str->size > MY_STRING_INITIAL_SIZE) {
        str->capacity = MY_STRING_RESIZE_POLICIE(str->size);

        char* data = NULL;
        MY_CALLOC(data, char, str->capacity + 1);
        memcpy(data, str->data, str->size);

        MY_FREE(str->data);
        str->data = data;
        str->data[str->size] = '\0';
    }
}
void        MyString_Clear      (MyString* str) {
    MY_ASSERT_PTR(str);

    str->size = 0;
    str->data[0] = '\0';
}

void        MyString_Erase      (MyString* str, size_t index) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(index, str->size);

    if (index != str->size - 1) {
        memmove(&str->data[index], &str->data[index + 1], (str->size - index - 1));
    }

    str->size--;
    str->data[str->size] = '\0';
    MyString_Shrink(str);
}
void        MyString_PopBack    (MyString* str) {
    MY_ASSERT_PTR(str);

    if (str->size == 0) {
        return;
    }

    MyString_Erase(str, str->size - 1);
}
void        MyString_PopFront   (MyString* str) {
    MY_ASSERT_PTR(str);

    if (str->size == 0) {
        return;
    }

    MyString_Erase(str, 0);
}

void        MyString_Insert     (MyString* str, size_t index, char ch) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(index, str->size + 1);

    if (str->size != str->capacity) {
        if (index != str->size) {
            memmove(&str->data[index + 1], &str->data[index], (str->size - index));
        }
        goto end;
    }

    str->capacity = MY_STRING_RESIZE_POLICIE(str->size);
    char* data = NULL;
    MY_CALLOC(data, char, str->capacity + 1);

    if (index != 0) {
        memcpy(data, str->data, index);
    }

    if (index != str->size) {
        memcpy(&data[index + 1], &str->data[index], (str->size - index));
    }

    MY_FREE(str->data);
    str->data = data;

end:
    str->size++;
    str->data[index] = ch;
    str->data[str->size] = '\0';
}
void        MyString_PushBack   (MyString* str, char ch) {
    MY_ASSERT_PTR(str);
    MyString_Insert(str, str->size, ch);
}
void        MyString_PushFront  (MyString* str, char ch) {
    MY_ASSERT_PTR(str);
    MyString_Insert(str, 0, ch);
}

void        MyString_Memset     (MyString* str, char ch, size_t start, size_t count) {
    MY_ASSERT_PTR(str);
    if (count == 0) { return; }

    MY_ASSERT_BOUNDS(start, str->size);
    MY_ASSERT_BOUNDS(start + (count - 1), str->size);
    memset(&str->data[start], ch, count);
    str->data[str->size] = '\0';
}
void        MyString_Memcpy     (MyString* str, const void* src, size_t start, size_t count) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_PTR(src);
    if (count == 0) { return; }

    MY_ASSERT_BOUNDS(start, str->size);
    MY_ASSERT_BOUNDS(start + (count - 1), str->size);
    memcpy(&str->data[start], src, count);
    str->data[str->size] = '\0';
}

void        MyString_AppendCh   (MyString* str, char ch, size_t count) {
    MY_ASSERT_PTR(str);
    if (count == 0) { return; }

    size_t size = str->size;
    MyString_Resize(str, size + count);
    MyString_Memset(str, ch, size, count);
}
void        MyString_AppendN    (MyString* str, const char* src, size_t count) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_PTR(src);
    if (count == 0) { return; }

    size_t size = str->size;
    MyString_Resize(str, size + count);
    MyString_Memcpy(str, src, size, count);
}
void        MyString_Append     (MyString* str, const char* src) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_PTR(src);

    size_t length = strlen(src);
    MyString_AppendN(str, src, length);
}

#ifdef __cplusplus
}
#endif