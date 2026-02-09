#ifndef __MYSTD_STRING_H__
#define __MYSTD_STRING_H__

#include <mystd/stdlib.h>

#ifndef MY_STRING_NAME
    #define MY_STRING_NAME MyString
#endif /* MY_STRING_NAME */

#ifndef MY_STRING_FN_PREFIX
    #define MY_STRING_FN_PREFIX MY_STRING_NAME
#endif /* MY_STRING_FN_PREFIX */

#ifndef MY_STRING_RESIZE_POLICIE
    #define MY_STRING_RESIZE_POLICIE(size) (size * 2)
#endif /* MY_STRING_RESIZE_POLICIE */

#ifndef MY_STRING_SHRINK_POLICIE
    #define MY_STRING_SHRINK_POLICIE(capacity) (capacity / 4)
#endif /* MY_STRING_SHRINK_POLICIE */

#ifndef MY_STRING_INITIAL_SIZE
    #define MY_STRING_INITIAL_SIZE 10
#endif /* MY_STRING_INITIAL_SIZE */

/** @cond doxygen_ignore */
#define MY_STRING_STRUCT         MY_STRING_NAME

#define MY_STRING_FN_CREATE      MY_CONCAT2(MY_STRING_FN_PREFIX, _create)
#define MY_STRING_FN_DESTROY     MY_CONCAT2(MY_STRING_FN_PREFIX, _destroy)

#define MY_STRING_FN_RDLOCK      MY_CONCAT2(MY_STRING_FN_PREFIX, _rdlock)
#define MY_STRING_FN_WRLOCK      MY_CONCAT2(MY_STRING_FN_PREFIX, _wrlock)
#define MY_STRING_FN_RDUNLOCK    MY_CONCAT2(MY_STRING_FN_PREFIX, _rdunlock)
#define MY_STRING_FN_WRUNLOCK    MY_CONCAT2(MY_STRING_FN_PREFIX, _wrunlock)

#define MY_STRING_FN_CSTR        MY_CONCAT2(MY_STRING_FN_PREFIX, _cstr)
#define MY_STRING_FN_SIZE        MY_CONCAT2(MY_STRING_FN_PREFIX, _size)

#define MY_STRING_FN_SET         MY_CONCAT2(MY_STRING_FN_PREFIX, _set)
#define MY_STRING_FN_GET         MY_CONCAT2(MY_STRING_FN_PREFIX, _get)
#define MY_STRING_FN_RESIZE      MY_CONCAT2(MY_STRING_FN_PREFIX, _resize)
#define MY_STRING_FN_SHRINK      MY_CONCAT2(MY_STRING_FN_PREFIX, _shrink)

#define MY_STRING_FN_CLEAR       MY_CONCAT2(MY_STRING_FN_PREFIX, _clear)
#define MY_STRING_FN_ERASE       MY_CONCAT2(MY_STRING_FN_PREFIX, _erase)
#define MY_STRING_FN_POP_BACK    MY_CONCAT2(MY_STRING_FN_PREFIX, _pop_back)
#define MY_STRING_FN_POP_FRONT   MY_CONCAT2(MY_STRING_FN_PREFIX, _pop_front)

#define MY_STRING_FN_INSERT      MY_CONCAT2(MY_STRING_FN_PREFIX, _insert)
#define MY_STRING_FN_PUSH_BACK   MY_CONCAT2(MY_STRING_FN_PREFIX, _push_back)
#define MY_STRING_FN_PUSH_FRONT  MY_CONCAT2(MY_STRING_FN_PREFIX, _push_front)

#define MY_STRING_FN_MEMCPY      MY_CONCAT2(MY_STRING_FN_PREFIX, _memcpy)

/** @endcond */

#ifdef __cplusplus
extern "C" {
#endif

struct MY_STRING_STRUCT;
typedef struct MY_STRING_STRUCT MY_STRING_STRUCT;

MY_STRING_STRUCT*  MY_STRING_FN_CREATE      (MY_STRING_STRUCT* str);
void               MY_STRING_FN_DESTROY     (MY_STRING_STRUCT* str);

void               MY_STRING_FN_RDLOCK      (MY_STRING_STRUCT* str);
void               MY_STRING_FN_WRLOCK      (MY_STRING_STRUCT* str);
void               MY_STRING_FN_RDUNLOCK    (MY_STRING_STRUCT* str);
void               MY_STRING_FN_WRUNLOCK    (MY_STRING_STRUCT* str);

char*              MY_STRING_FN_CSTR        (MY_STRING_STRUCT* str);
size_t             MY_STRING_FN_SIZE        (MY_STRING_STRUCT* str);

void               MY_STRING_FN_SET         (MY_STRING_STRUCT* str, size_t idx, char c);
char               MY_STRING_FN_GET         (MY_STRING_STRUCT* str, size_t idx);
void               MY_STRING_FN_RESIZE      (MY_STRING_STRUCT* str, size_t size);
void               MY_STRING_FN_SHRINK      (MY_STRING_STRUCT* str);

void               MY_STRING_FN_CLEAR       (MY_STRING_STRUCT* str);
void               MY_STRING_FN_ERASE       (MY_STRING_STRUCT* str, size_t idx);
void               MY_STRING_FN_POP_BACK    (MY_STRING_STRUCT* str);
void               MY_STRING_FN_POP_FRONT   (MY_STRING_STRUCT* str);

void               MY_STRING_FN_INSERT      (MY_STRING_STRUCT* str, size_t idx, char c);
void               MY_STRING_FN_PUSH_BACK   (MY_STRING_STRUCT* str, char c);
void               MY_STRING_FN_PUSH_FRONT  (MY_STRING_STRUCT* str, char c);

void               MY_STRING_FN_MEMCPY      (MY_STRING_STRUCT* str, size_t idx, const void* src, size_t count);

struct MY_STRING_STRUCT {
    char*             data;
    size_t            size;
    size_t            capacity;
    int               allocated;
    MY_RWLOCK_TYPE    lock;
};

#ifdef MY_STRING_IMPLEMENTATION

MY_STRING_STRUCT* MY_STRING_FN_CREATE(MY_STRING_STRUCT* str) {
    if (!str) {
        MY_CALLOC(str, MY_STRING_STRUCT, 1);
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
void MY_STRING_FN_DESTROY(MY_STRING_STRUCT* str) {
    MY_ASSERT_PTR(str);

    MY_RWLOCK_DESTROY(str->lock);
    MY_FREE(str->data);

    if (str->allocated) {
        MY_FREE(str);
    }
}

void MY_STRING_FN_RDLOCK(MY_STRING_STRUCT* str) {
    MY_ASSERT_PTR(str);
    MY_RWLOCK_RDLOCK(str->lock);
}
void MY_STRING_FN_WRLOCK(MY_STRING_STRUCT* str) {
    MY_ASSERT_PTR(str);
    MY_RWLOCK_WRLOCK(str->lock);
}
void MY_STRING_FN_RDUNLOCK(MY_STRING_STRUCT* str) {
    MY_ASSERT_PTR(str);
    MY_RWLOCK_RDUNLOCK(str->lock);
}
void MY_STRING_FN_WRUNLOCK(MY_STRING_STRUCT* str) {
    MY_ASSERT_PTR(str);
    MY_RWLOCK_WRUNLOCK(str->lock);
}

char* MY_STRING_FN_CSTR(MY_STRING_STRUCT* str) {
    MY_ASSERT_PTR(str);
    return str->data;
}
size_t MY_STRING_FN_SIZE(MY_STRING_STRUCT* str) {
    MY_ASSERT_PTR(str);
    return str->size;
}

void MY_STRING_FN_SET(MY_STRING_STRUCT* str, size_t idx, char c) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(idx, str->size);

    str->data[idx] = c;
}
char MY_STRING_FN_GET(MY_STRING_STRUCT* str, size_t idx) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(idx, str->size);

    return str->data[idx];
}
void MY_STRING_FN_RESIZE(MY_STRING_STRUCT* str, size_t size) {
    MY_ASSERT_PTR(str);

    if (size == str->size) {
        return;
    }

    if (size == 0) {
        MY_STRING_FN_CLEAR(str);
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
void MY_STRING_FN_SHRINK(MY_STRING_STRUCT* str) {
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
void MY_STRING_FN_CLEAR(MY_STRING_STRUCT* str) {
    MY_ASSERT_PTR(str);

    str->size = 0;
    str->capacity = MY_STRING_INITIAL_SIZE;
    MY_FREE(str->data);
    MY_CALLOC(str->data, char, str->capacity + 1);
}

void MY_STRING_FN_ERASE(MY_STRING_STRUCT* str, size_t idx) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_BOUNDS(idx, str->size);

    if (idx != str->size - 1) {
        memmove(&str->data[idx], &str->data[idx + 1], (str->size - idx - 1));
    }

    str->size--;
    str->data[str->size] = '\0';
    MY_STRING_FN_SHRINK(str);
}
void MY_STRING_FN_POP_BACK(MY_STRING_STRUCT* str) {
    MY_ASSERT_PTR(str);

    if (str->size == 0) {
        MY_EMPTY_POPPING();
        return;
    }

    MY_STRING_FN_ERASE(str, str->size - 1);
}
void MY_STRING_FN_POP_FRONT(MY_STRING_STRUCT* str) {
    MY_ASSERT_PTR(str);

    if (str->size == 0) {
        MY_EMPTY_POPPING();
        return;
    }

    MY_STRING_FN_ERASE(str, 0);
}

void MY_STRING_FN_INSERT(MY_STRING_STRUCT* str, size_t idx, char c) {
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
void MY_STRING_FN_PUSH_BACK(MY_STRING_STRUCT* str, char c) {
    MY_ASSERT_PTR(str);

    MY_STRING_FN_INSERT(str, str->size, c);
}
void MY_STRING_FN_PUSH_FRONT(MY_STRING_STRUCT* str, char c) {
    MY_ASSERT_PTR(str);

    MY_STRING_FN_INSERT(str, 0, c);
}

void MY_STRING_FN_MEMCPY(MY_STRING_STRUCT* str, size_t idx, const void* src, size_t count) {
    MY_ASSERT_PTR(str);
    MY_ASSERT_PTR(src);

    if (count == 0) {
        return;
    }

    MY_ASSERT_BOUNDS(idx, str->size);
    MY_ASSERT_BOUNDS(idx + count, str->size + 1);

    memcpy(&str->data[idx], src, count);
}

#endif /* MY_STRING_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* __MYSTD_STRING_H__ */