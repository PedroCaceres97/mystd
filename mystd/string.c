#include <mystd/string.h>

MyString* MyString_Create(MyString* str) {
    if (!str) {
        MY_CALLOC(str, MyString, 1);
        str->allocated = true;
    } else {
        str->allocated = false;
    }

    str->size = 0;
    str->capacity = MY_STRING_INITIAL_SIZE;
    MY_RWLOCK_INIT(str->lock);
    MY_CALLOC(str->data, char, str->capacity + 1);

    return str;
}
void MyString_Destroy(MyString* str) {
    MY_ASSERT_PTR(str);

    MY_RWLOCK_DESTROY(str->lock);
    MY_FREE(str->data);

    if (str->allocated) {
        MY_FREE(str);
    }
}

void MyString_Rdlock(MyString* str) {
    MY_ASSERT_PTR(str);
    MY_RWLOCK_RDLOCK(str->lock);
}
void MyString_Wrlock(MyString* str) {
    MY_ASSERT_PTR(str);
    MY_RWLOCK_WRLOCK(str->lock);
}
void MyString_Rdunlock(MyString* str) {
    MY_ASSERT_PTR(str);
    MY_RWLOCK_RDUNLOCK(str->lock);
}
void MyString_Wrunlock(MyString* str) {
    MY_ASSERT_PTR(str);
    MY_RWLOCK_WRUNLOCK(str->lock);
}

char* MyString_Cstr(MyString* str) {
    MY_ASSERT_PTR(str);
    return str->data;
}
size_t MyString_Size(MyString* str) {
    MY_ASSERT_PTR(str);
    return str->size;
}

void MyString_Set(MyString* str, size_t idx, char c) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(idx, str->size);

    str->data[idx] = c;
}
char MyString_Get(MyString* str, size_t idx) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(idx, str->size);

    return str->data[idx];
}
void MyString_Resize(MyString* str, size_t size) {
    MY_ASSERT_PTR(str);

    if (size == str->size) {
        return;
    }

    if (size == 0) {
        MyString_Clear(str);
        return;
    }

    str->capacity = MY_TERNARY(
        size < MY_STRING_INITIAL_SIZE,
        MY_STRING_INITIAL_SIZE,
        MY_STRING_RESIZE_POLICIE(size)
    );

    char* data = NULL;
    MY_CALLOC(data, char, str->capacity + 1);
    memcpy(data, str->data, MY_MIN(str->size, size));

    MY_FREE(str->data);
    str->data = data;
    str->size = size;
    str->data[str->size] = '\0';
}
void MyString_Shrink(MyString* str) {
    MY_ASSERT_PTR(str);

    if (str->size < MY_STRING_SHRINK_POLICIE(str->capacity) && str->size < MY_STRING_INITIAL_SIZE) {
        str->capacity = MY_STRING_RESIZE_POLICIE(str->size);

        char* data = NULL;
        MY_CALLOC(data, char, str->capacity + 1);
        memcpy(data, str->data, str->size);

        MY_FREE(str->data);
        str->data = data;
        str->data[str->size] = '\0';
    }
}
void MyString_Clear(MyString* str) {
    MY_ASSERT_PTR(str);

    str->size = 0;
    str->capacity = MY_STRING_INITIAL_SIZE;
    MY_FREE(str->data);
    MY_CALLOC(str->data, char, str->capacity + 1);
}

void MyString_Erase(MyString* str, size_t idx) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(idx, str->size);

    if (idx != str->size - 1) {
        memmove(&str->data[idx], &str->data[idx + 1], (str->size - idx - 1));
    }

    str->size--;
    str->data[str->size] = '\0';
    MyString_Shrink(str);
}
void MyString_PopBack(MyString* str) {
    MY_ASSERT_PTR(str);

    if (str->size == 0) {
        MY_EMPTY_POPPING();
        return;
    }

    MyString_Erase(str, str->size - 1);
}
void MyString_PopFront(MyString* str) {
    MY_ASSERT_PTR(str);

    if (str->size == 0) {
        MY_EMPTY_POPPING();
        return;
    }

    MyString_Erase(str, 0);
}

void MyString_Insert(MyString* str, size_t idx, char c) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(idx, str->size + 1);

    if (str->size != str->capacity) {
        if (idx != str->size) {
            memmove(&str->data[idx + 1], &str->data[idx], (str->size - idx));
        }
    } else {
        str->capacity = MY_STRING_RESIZE_POLICIE(str->size);
        char* data = NULL;
        MY_CALLOC(data, char, str->capacity + 1);

        if (idx != 0) {
            memcpy(data, str->data, idx);
        }

        if (idx != str->size) {
            memcpy(&data[idx + 1], &str->data[idx], (str->size - idx));
        }

        MY_FREE(str->data);
        str->data = data;
    }

    str->size++;
    str->data[idx] = c;
    str->data[str->size] = '\0';
}
void MyString_PushBack(MyString* str, char c) {
    MY_ASSERT_PTR(str);

    MyString_Insert(str, str->size, c);
}
void MyString_PushFront(MyString* str, char c) {
    MY_ASSERT_PTR(str);

    MyString_Insert(str, 0, c);
}

void MyString_Memcpy(MyString* str, size_t idx, const void* src, size_t count) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_PTR(src);

    if (count == 0) {
        return;
    }

    MY_ASSERT_BOUNDS(idx, str->size);
    MY_ASSERT_BOUNDS(idx + count, str->size + 1);

    memcpy(&str->data[idx], src, count);
}