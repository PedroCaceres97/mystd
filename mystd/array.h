#include <mystd/stdlib.h>

#ifndef MY_ARRAY_NAME
    #define MY_ARRAY_NAME MyArrayInt
#endif /* MY_ARRAY_NAME */

#ifndef MY_ARRAY_FN_PREFIX
    #define MY_ARRAY_FN_PREFIX MY_ARRAY_NAME
#endif /* MY_ARRAY_FN_PREFIX */

#ifndef MY_ARRAY_DATA_TYPE
    #define MY_ARRAY_DATA_TYPE int
#endif /* MY_ARRAY_DATA_TYPE */

#ifndef MY_ARRAY_SIZE
    #define MY_ARRAY_SIZE 10
#endif /* MY_ARRAY_SIZE */

/** @cond doxygen_ignore */
#define MY_ARRAY_STRUCT      MY_ARRAY_NAME

#define MY_ARRAY_FN_CREATE      MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Create)
#define MY_ARRAY_FN_DESTROY     MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Destroy)

#define MY_ARRAY_FN_RDLOCK      MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Rdlock)
#define MY_ARRAY_FN_WRLOCK      MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Wrlock)
#define MY_ARRAY_FN_RDUNLOCK    MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Rdunlock)
#define MY_ARRAY_FN_WRUNLOCK    MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Wrunlock)

#define MY_ARRAY_FN_DATA        MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Data)
#define MY_ARRAY_FN_SIZE        MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Size)
#define MY_ARRAY_FN_BACK        MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Back)
#define MY_ARRAY_FN_FRONT       MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Front)

#define MY_ARRAY_FN_SET         MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Set)
#define MY_ARRAY_FN_GET         MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Get)

#define MY_ARRAY_FN_CLEAR       MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Clear)
#define MY_ARRAY_FN_ERASE       MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Erase)
#define MY_ARRAY_FN_POP_BACK    MY_CONCAT2(MY_ARRAY_FN_PREFIX, _PopBack)
#define MY_ARRAY_FN_POP_FRONT   MY_CONCAT2(MY_ARRAY_FN_PREFIX, _PopFront)

#define MY_ARRAY_FN_INSERT      MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Insert)
#define MY_ARRAY_FN_PUSH_BACK   MY_CONCAT2(MY_ARRAY_FN_PREFIX, _PushBack)
#define MY_ARRAY_FN_PUSH_FRONT  MY_CONCAT2(MY_ARRAY_FN_PREFIX, _PushFront)

#define MY_ARRAY_FN_MEMCPY      MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Memcpy)
#define MY_ARRAY_FN_MEMSET      MY_CONCAT2(MY_ARRAY_FN_PREFIX, _Memset)
/** @endcond */

#ifdef __cplusplus
extern "C" {
#endif

struct MY_ARRAY_STRUCT;
typedef struct MY_ARRAY_STRUCT MY_ARRAY_STRUCT;

MY_ARRAY_STRUCT*    MY_ARRAY_FN_CREATE    (MY_ARRAY_STRUCT* array);
void                MY_ARRAY_FN_DESTROY   (MY_ARRAY_STRUCT* array);

void                MY_ARRAY_FN_RDLOCK    (MY_ARRAY_STRUCT* array);
void                MY_ARRAY_FN_WRLOCK    (MY_ARRAY_STRUCT* array);
void                MY_ARRAY_FN_RDUNLOCK  (MY_ARRAY_STRUCT* array);
void                MY_ARRAY_FN_WRUNLOCK  (MY_ARRAY_STRUCT* array);

MY_ARRAY_DATA_TYPE* MY_ARRAY_FN_DATA      (MY_ARRAY_STRUCT* array);
size_t              MY_ARRAY_FN_SIZE      (MY_ARRAY_STRUCT* array);
MY_ARRAY_DATA_TYPE  MY_ARRAY_FN_BACK      (MY_ARRAY_STRUCT* array);
MY_ARRAY_DATA_TYPE  MY_ARRAY_FN_FRONT     (MY_ARRAY_STRUCT* array);

void                MY_ARRAY_FN_SET       (MY_ARRAY_STRUCT* array, size_t idx,       MY_ARRAY_DATA_TYPE value);
MY_ARRAY_DATA_TYPE  MY_ARRAY_FN_GET       (MY_ARRAY_STRUCT* array, size_t idx);

void                MY_ARRAY_FN_CLEAR     (MY_ARRAY_STRUCT* array);
void                MY_ARRAY_FN_ERASE     (MY_ARRAY_STRUCT* array, size_t idx);
void                MY_ARRAY_FN_POP_BACK  (MY_ARRAY_STRUCT* array);
void                MY_ARRAY_FN_POP_FRONT (MY_ARRAY_STRUCT* array);

void                MY_ARRAY_FN_INSERT    (MY_ARRAY_STRUCT* array, size_t idx,       MY_ARRAY_DATA_TYPE value);
void                MY_ARRAY_FN_PUSH_BACK (MY_ARRAY_STRUCT* array,                   MY_ARRAY_DATA_TYPE value);
void                MY_ARRAY_FN_PUSH_FRONT(MY_ARRAY_STRUCT* array,                   MY_ARRAY_DATA_TYPE value);

void                MY_ARRAY_FN_MEMCPY    (MY_ARRAY_STRUCT* array, size_t idx, const MY_ARRAY_DATA_TYPE* src,  size_t count);
void                MY_ARRAY_FN_MEMSET    (MY_ARRAY_STRUCT* array, size_t idx,       MY_ARRAY_DATA_TYPE value, size_t count);

struct MY_ARRAY_STRUCT {
    MY_ARRAY_DATA_TYPE  data[MY_ARRAY_SIZE];
    size_t              size;
    int                 allocated;
    MY_RWLOCK_TYPE      lock;
};

#ifdef MY_ARRAY_IMPLEMENTATION

MY_ARRAY_STRUCT*    MY_ARRAY_FN_CREATE(MY_ARRAY_STRUCT* array) {
  if (!array) {
    MY_CALLOC(array, struct MY_ARRAY_STRUCT, 1);
    array->allocated = true;
  } else {
    array->allocated = false;
  }

  array->size = 0;
  memset(array->data, 0, sizeof(array->data));
  MY_RWLOCK_INIT(array->lock);

  return array;
}
void                MY_ARRAY_FN_DESTROY(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);
  MY_ASSERT(array->size == 0, "Destroying non empty array (HINT: Clear the array)");

  MY_RWLOCK_DESTROY(array->lock);

  if (array->allocated) {
    MY_FREE(array);
  }
}

void                MY_ARRAY_FN_RDLOCK(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);
  MY_RWLOCK_RDLOCK(array->lock);
}
void                MY_ARRAY_FN_WRLOCK(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);
  MY_RWLOCK_WRLOCK(array->lock);
}
void                MY_ARRAY_FN_RDUNLOCK(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);
  MY_RWLOCK_RDUNLOCK(array->lock);
}
void                MY_ARRAY_FN_WRUNLOCK(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);
  MY_RWLOCK_WRUNLOCK(array->lock);
}

MY_ARRAY_DATA_TYPE* MY_ARRAY_FN_DATA(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);
  return array->data;
}
size_t              MY_ARRAY_FN_SIZE(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);
  return array->size;
}
MY_ARRAY_DATA_TYPE  MY_ARRAY_FN_BACK(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);
  MY_ASSERT(array->size != 0, "ARRAY has no back (size == 0)");
  return array->data[array->size - 1];
}
MY_ARRAY_DATA_TYPE  MY_ARRAY_FN_FRONT(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);
  MY_ASSERT(array->size != 0, "ARRAY has no front (size == 0)");
  return array->data[0];
}

void                MY_ARRAY_FN_SET(MY_ARRAY_STRUCT* array, size_t idx, MY_ARRAY_DATA_TYPE value) {
  MY_ASSERT_PTR(array);
  MY_ASSERT_BOUNDS(idx, array->size);
  array->data[idx] = value;
}
MY_ARRAY_DATA_TYPE  MY_ARRAY_FN_GET(MY_ARRAY_STRUCT* array, size_t idx) {
  MY_ASSERT_PTR(array);
  MY_ASSERT_BOUNDS(idx, array->size);
  return array->data[idx];
}

void                MY_ARRAY_FN_CLEAR(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);

  #ifdef MY_ARRAY_DEALLOCATE_DATA
    for (size_t i = 0; i < array->size; i++) {
      MY_ARRAY_DEALLOCATE_DATA(array->data[i]);
    }
  #endif /* MY_ARRAY_DEALLOCATE_DATA */

  array->size = 0;
  memset(array->data, 0, sizeof(array->data));
}
void                MY_ARRAY_FN_ERASE(MY_ARRAY_STRUCT* array, size_t idx) {
  MY_ASSERT_PTR(array);
  MY_ASSERT_BOUNDS(idx, array->size);

  #ifdef MY_ARRAY_DEALLOCATE_DATA
    MY_ARRAY_DEALLOCATE_DATA(array->data[idx]);
  #endif /* MY_ARRAY_DEALLOCATE_DATA */

  if (idx != array->size - 1) {
    memmove(&array->data[idx], &array->data[idx + 1], (array->size - idx - 1) * sizeof(MY_ARRAY_DATA_TYPE));
  }
  
  array->size--;
}
void                MY_ARRAY_FN_POP_BACK(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);

  if (array->size == 0) {
    MY_EMPTY_POPPING();
    return;
  }

  MY_ARRAY_FN_ERASE(array, array->size - 1);
}
void                MY_ARRAY_FN_POP_FRONT(MY_ARRAY_STRUCT* array) {
  MY_ASSERT_PTR(array);

  if (array->size == 0) {
    MY_EMPTY_POPPING();
    return;
  }

  MY_ARRAY_FN_ERASE(array, 0);
}

void                MY_ARRAY_FN_INSERT(MY_ARRAY_STRUCT* array, size_t idx, MY_ARRAY_DATA_TYPE value) {
    MY_ASSERT_PTR(array);
    MY_ASSERT_BOUNDS(idx, array->size + 1);

    if (idx != array->size) {
        memmove(&array->data[idx + 1], &array->data[idx], (array->size - idx) * sizeof(MY_ARRAY_DATA_TYPE));
    }

    array->data[idx] = value;
    array->size++;
}
void                MY_ARRAY_FN_PUSH_BACK(MY_ARRAY_STRUCT* array, MY_ARRAY_DATA_TYPE value) {
  MY_ASSERT_PTR(array);
  MY_ARRAY_FN_INSERT(array, array->size, value);
}
void                MY_ARRAY_FN_PUSH_FRONT(MY_ARRAY_STRUCT* array, MY_ARRAY_DATA_TYPE value) {
  MY_ASSERT_PTR(array);
  MY_ARRAY_FN_INSERT(array, 0, value);
}

void                MY_ARRAY_FN_MEMCPY(MY_ARRAY_STRUCT* array, size_t idx, const MY_ARRAY_DATA_TYPE* src, size_t count) {
    MY_ASSERT_PTR(array);
    MY_ASSERT_PTR(src);

    if (count == 0) { return; }

    MY_ASSERT_BOUNDS(idx, array->size);
    MY_ASSERT_BOUNDS(idx + count, array->size + 1);

    memcpy(&array->data[idx], src, count * sizeof(MY_ARRAY_DATA_TYPE));
}
void                MY_ARRAY_FN_MEMSET(MY_ARRAY_STRUCT* array, size_t idx, MY_ARRAY_DATA_TYPE value, size_t count) {
    MY_ASSERT_PTR(array);

    if (count == 0) { return; }

    MY_ASSERT_BOUNDS(idx, array->size);
    MY_ASSERT_BOUNDS(idx + count, array->size + 1);

    for (size_t i = 0; i < count; i++) {
        array->data[idx + i] = value;
    }
}

#endif /* MY_ARRAY_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#undef MY_ARRAY_NAME
#undef MY_ARRAY_FN_PREFIX

#undef MY_ARRAY_DATA_TYPE
#undef MY_ARRAY_INITIAL_SIZE
#undef MY_ARRAY_SHRINK_POLICIE
#undef MY_ARRAY_RESIZE_POLICIE
#undef MY_ARRAY_DEALLOCATE_DATA
#undef MY_ARRAY_IMPLEMENTATION
 
#undef MY_ARRAY_STRUCT

#undef MY_ARRAY_CREATE    
#undef MY_ARRAY_DESTROY

#undef MY_ARRAY_RDLOCK    
#undef MY_ARRAY_WRLOCK    
#undef MY_ARRAY_RDUNLOCK  
#undef MY_ARRAY_WRUNLOCK

#undef MY_ARRAY_DATA      
#undef MY_ARRAY_SIZE      
#undef MY_ARRAY_BACK      
#undef MY_ARRAY_FRONT

#undef MY_ARRAY_SET
#undef MY_ARRAY_GET

#undef MY_ARRAY_CLEAR     
#undef MY_ARRAY_ERASE     
#undef MY_ARRAY_POP_BACK  
#undef MY_ARRAY_POP_FRONT

#undef MY_ARRAY_INSERT    
#undef MY_ARRAY_PUSH_BACK 
#undef MY_ARRAY_PUSH_FRONT

#undef MY_ARRAY_MEMCPY    
#undef MY_ARRAY_MEMSET