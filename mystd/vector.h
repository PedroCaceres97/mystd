#include <mystd/stdlib.h>

#ifndef MY_VECTOR_NAME
  #define MY_VECTOR_NAME MyVecInt
#endif /* MY_VECTOR_NAME */

#ifndef MY_VECTOR_FN_PREFIX
  #define MY_VECTOR_FN_PREFIX MY_VECTOR_NAME
#endif /* MY_VECTOR_FN_PREFIX */

#ifndef MY_VECTOR_DATA_TYPE
  #define MY_VECTOR_DATA_TYPE int
#endif /* MY_VECTOR_DATA_TYPE */

#ifndef MY_VECTOR_INITIAL_SIZE
  #define MY_VECTOR_INITIAL_SIZE 10
#endif /* MY_VECTOR_INITIAL_SIZE */

#ifndef MY_VECTOR_RESIZE_POLICIE
  #define MY_VECTOR_RESIZE_POLICIE(size) (size * 2)
#endif /* MY_VECTOR_RESIZE_POLICIE */

#ifndef MY_VECTOR_SHRINK_POLICIE
  #define MY_VECTOR_SHRINK_POLICIE(capacity) (capacity / 4)
#endif /* MY_VECTOR_SHRINK_POLICIE */

/** @cond doxygen_ignore */
#define MY_VECTOR_STRUCT        MY_VECTOR_NAME

#define MY_VECTOR_FN_CREATE     MY_CONCAT2(MY_VECTOR_FN_PREFIX, Create)
#define MY_VECTOR_FN_DESTROY    MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Destroy)

#define MY_VECTOR_FN_RDLOCK     MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Rdlock)
#define MY_VECTOR_FN_WRLOCK     MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Wrlock)
#define MY_VECTOR_FN_RDUNLOCK   MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Rdunlock)
#define MY_VECTOR_FN_WRUNLOCK   MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Wrunlock)

#define MY_VECTOR_FN_DATA       MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Data)
#define MY_VECTOR_FN_SIZE       MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Size)
#define MY_VECTOR_FN_BACK       MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Back)
#define MY_VECTOR_FN_FRONT      MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Front)

#define MY_VECTOR_FN_SET        MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Set)
#define MY_VECTOR_FN_GET        MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Get)
#define MY_VECTOR_FN_RESIZE     MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Resize)
#define MY_VECTOR_FN_SHRINK     MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Shrink)

#define MY_VECTOR_FN_CLEAR      MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Clear)
#define MY_VECTOR_FN_ERASE      MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Erase)
#define MY_VECTOR_FN_POP_BACK   MY_CONCAT2(MY_VECTOR_FN_PREFIX, _PopBack)
#define MY_VECTOR_FN_POP_FRONT  MY_CONCAT2(MY_VECTOR_FN_PREFIX, _PopFront)

#define MY_VECTOR_FN_INSERT     MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Insert)
#define MY_VECTOR_FN_PUSH_BACK  MY_CONCAT2(MY_VECTOR_FN_PREFIX, _PushBack)
#define MY_VECTOR_FN_PUSH_FRONT MY_CONCAT2(MY_VECTOR_FN_PREFIX, _PushFront)

#define MY_VECTOR_FN_MEMCPY     MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Memcpy)
#define MY_VECTOR_FN_MEMSET     MY_CONCAT2(MY_VECTOR_FN_PREFIX, _Memset)
/** @endcond */

#ifdef __cplusplus
extern "C" {
#endif

struct MY_VECTOR_STRUCT;
typedef struct MY_VECTOR_STRUCT MY_VECTOR_STRUCT;

MY_VECTOR_STRUCT*       MY_VECTOR_FN_CREATE     (MY_VECTOR_STRUCT* vec);
void                    MY_VECTOR_FN_DESTROY    (MY_VECTOR_STRUCT* vec);

void                    MY_VECTOR_FN_RDLOCK     (MY_VECTOR_STRUCT* vec);
void                    MY_VECTOR_FN_WRLOCK     (MY_VECTOR_STRUCT* vec);
void                    MY_VECTOR_FN_RDUNLOCK   (MY_VECTOR_STRUCT* vec);
void                    MY_VECTOR_FN_WRUNLOCK   (MY_VECTOR_STRUCT* vec);

MY_VECTOR_DATA_TYPE*    MY_VECTOR_FN_DATA       (MY_VECTOR_STRUCT* vec);
size_t                  MY_VECTOR_FN_SIZE       (MY_VECTOR_STRUCT* vec);
MY_VECTOR_DATA_TYPE     MY_VECTOR_FN_BACK       (MY_VECTOR_STRUCT* vec);
MY_VECTOR_DATA_TYPE     MY_VECTOR_FN_FRONT      (MY_VECTOR_STRUCT* vec);

void                    MY_VECTOR_FN_SET        (MY_VECTOR_STRUCT* vec, size_t idx,        MY_VECTOR_DATA_TYPE value);
MY_VECTOR_DATA_TYPE     MY_VECTOR_FN_GET        (MY_VECTOR_STRUCT* vec, size_t idx);
void                    MY_VECTOR_FN_RESIZE     (MY_VECTOR_STRUCT* vec, size_t size);
void                    MY_VECTOR_FN_SHRINK     (MY_VECTOR_STRUCT* vec);

void                    MY_VECTOR_FN_CLEAR      (MY_VECTOR_STRUCT* vec);
void                    MY_VECTOR_FN_ERASE      (MY_VECTOR_STRUCT* vec, size_t idx);
void                    MY_VECTOR_FN_POP_BACK   (MY_VECTOR_STRUCT* vec);
void                    MY_VECTOR_FN_POP_FRONT  (MY_VECTOR_STRUCT* vec);

void                    MY_VECTOR_FN_INSERT     (MY_VECTOR_STRUCT* vec, size_t idx,        MY_VECTOR_DATA_TYPE value);
void                    MY_VECTOR_FN_PUSH_BACK  (MY_VECTOR_STRUCT* vec,                    MY_VECTOR_DATA_TYPE value);
void                    MY_VECTOR_FN_PUSH_FRONT (MY_VECTOR_STRUCT* vec,                    MY_VECTOR_DATA_TYPE value);

void                    MY_VECTOR_FN_MEMCPY     (MY_VECTOR_STRUCT* vec, size_t idx, const  MY_VECTOR_DATA_TYPE* src,  size_t count);
void                    MY_VECTOR_FN_MEMSET     (MY_VECTOR_STRUCT* vec, size_t idx,        MY_VECTOR_DATA_TYPE value, size_t count);

struct MY_VECTOR_STRUCT {
    MY_VECTOR_DATA_TYPE*    data;
    size_t                  size;
    size_t                  capacity;
    int                     allocated;
    MY_RWLOCK_TYPE          lock;
};

#ifdef MY_VECTOR_IMPLEMENTATION

MY_VECTOR_STRUCT*       MY_VECTOR_FN_CREATE     (MY_VECTOR_STRUCT* vec) {
  if (!vec) {
    MY_CALLOC(vec, struct MY_VECTOR_STRUCT, 1);
    vec->allocated = true;
  } else {
    vec->allocated = false;
  }

  vec->size = 0;
  vec->capacity = MY_VECTOR_INITIAL_SIZE;
  MY_CALLOC(vec->data, MY_VECTOR_DATA_TYPE, vec->capacity);
  MY_RWLOCK_INIT(vec->lock);

  return vec;
}
void                    MY_VECTOR_FN_DESTROY    (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);
  MY_ASSERT(vec->size == 0, "Destroying non empty MY_VECTOR (HINT: Clear the MY_VECTOR)");

  MY_RWLOCK_DESTROY(vec->lock);
  MY_FREE(vec->data);

  if (vec->allocated) {
    MY_FREE(vec);
  }
}

void                    MY_VECTOR_FN_RDLOCK     (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);
  MY_RWLOCK_RDLOCK(vec->lock);
}
void                    MY_VECTOR_FN_WRLOCK     (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);
  MY_RWLOCK_WRLOCK(vec->lock);
}
void                    MY_VECTOR_FN_RDUNLOCK   (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);
  MY_RWLOCK_RDUNLOCK(vec->lock);
}
void                    MY_VECTOR_FN_WRUNLOCK   (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);
  MY_RWLOCK_WRUNLOCK(vec->lock);
}

MY_VECTOR_DATA_TYPE*    MY_VECTOR_FN_DATA       (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);
  return vec->data;
}
size_t                  MY_VECTOR_FN_SIZE       (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);
  return vec->size;
}
MY_VECTOR_DATA_TYPE     MY_VECTOR_FN_BACK       (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);
  MY_ASSERT(vec->size != 0, "MY_VECTOR has no back (size == 0)");
  return vec->data[vec->size - 1];
}
MY_VECTOR_DATA_TYPE     MY_VECTOR_FN_FRONT      (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);
  MY_ASSERT(vec->size != 0, "MY_VECTOR has no front (size == 0)");
  return vec->data[0];
}

void                    MY_VECTOR_FN_SET        (MY_VECTOR_STRUCT* vec, size_t idx, MY_VECTOR_DATA_TYPE value) {
  MY_ASSERT_PTR(vec);
  MY_ASSERT_BOUNDS(idx, vec->size);
  vec->data[idx] = value;
}
MY_VECTOR_DATA_TYPE     MY_VECTOR_FN_GET        (MY_VECTOR_STRUCT* vec, size_t idx) {
  MY_ASSERT_PTR(vec);
  MY_ASSERT_BOUNDS(idx, vec->size);
  return vec->data[idx];
}
void                    MY_VECTOR_FN_RESIZE     (MY_VECTOR_STRUCT* vec, size_t size) {
  MY_ASSERT_PTR(vec);

  if (size == vec->size) {
    return;
  }

  if (size == 0) {
    MY_VECTOR_FN_CLEAR(vec);
    return;
  }

  if (size < vec->size) {
    #ifdef MY_VECTOR_DEALLOCATE_DATA
      for (size_t i = size; i < vec->size; i++) {
        MY_VECTOR_DEALLOCATE_DATA(vec->data[i]);
      }
    #endif /* MY_VECTOR_DEALLOCATE_DATA */
  }

  vec->capacity = MY_TERNARY(
    size < MY_VECTOR_INITIAL_SIZE,
    MY_VECTOR_INITIAL_SIZE,
    MY_VECTOR_RESIZE_POLICIE(size)
  );

  MY_VECTOR_DATA_TYPE* data = NULL;
  MY_CALLOC(data, MY_VECTOR_DATA_TYPE, vec->capacity);
  memcpy(data, vec->data, MY_MIN(vec->size, size) * sizeof(MY_VECTOR_DATA_TYPE));

  MY_FREE(vec->data);
  vec->data = data;
  vec->size = size;
}
void                    MY_VECTOR_FN_SHRINK     (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);

  if (vec->size < MY_VECTOR_SHRINK_POLICIE(vec->capacity) && vec->size < MY_VECTOR_INITIAL_SIZE) {
    vec->capacity = MY_VECTOR_RESIZE_POLICIE(vec->size);
    MY_VECTOR_DATA_TYPE* data = NULL;
    MY_CALLOC(data, MY_VECTOR_DATA_TYPE, vec->capacity);

    memcpy(data, vec->data, vec->size * sizeof(MY_VECTOR_DATA_TYPE));
    MY_FREE(vec->data);
    vec->data = data;
  }
}

void                    MY_VECTOR_FN_CLEAR      (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);

  #ifdef MY_VECTOR_DEALLOCATE_DATA
    for (size_t i = 0; i < vec->size; i++) {
      MY_VECTOR_DEALLOCATE_DATA(vec->data[i]);
    }
  #endif /* MY_VECTOR_DEALLOCATE_DATA */

  vec->size = 0;
  vec->capacity = MY_VECTOR_INITIAL_SIZE;
  MY_FREE(vec->data);
  MY_CALLOC(vec->data, MY_VECTOR_DATA_TYPE, vec->capacity);
}
void                    MY_VECTOR_FN_ERASE      (MY_VECTOR_STRUCT* vec, size_t idx) {
  MY_ASSERT_PTR(vec);
  MY_ASSERT_BOUNDS(idx, vec->size);

  #ifdef MY_VECTOR_DEALLOCATE_DATA
    MY_VECTOR_DEALLOCATE_DATA(vec->data[idx]);
  #endif /* MY_VECTOR_DEALLOCATE_DATA */           

  if (idx != vec->size - 1) {
    memmove(&vec->data[idx], &vec->data[idx + 1], (vec->size - idx - 1) * sizeof(MY_VECTOR_DATA_TYPE));
  }
  
  vec->size--;

  MY_VECTOR_FN_SHRINK(vec);
}
void                    MY_VECTOR_FN_POP_BACK   (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);

  if (vec->size == 0) {
    MY_EMPTY_POPPING();
    return;
  }

  MY_VECTOR_FN_ERASE(vec, vec->size - 1);
}
void                    MY_VECTOR_FN_POP_FRONT  (MY_VECTOR_STRUCT* vec) {
  MY_ASSERT_PTR(vec);

  if (vec->size == 0) {
    MY_EMPTY_POPPING();
    return;
  }

  MY_VECTOR_FN_ERASE(vec, 0);
}

void                    MY_VECTOR_FN_INSERT     (MY_VECTOR_STRUCT* vec, size_t idx, MY_VECTOR_DATA_TYPE value) {
  MY_ASSERT_PTR(vec);
  MY_ASSERT_BOUNDS(idx, vec->size + 1);

  if (vec->size != vec->capacity) {
    if (idx != vec->size) {
      memmove(&vec->data[idx + 1], &vec->data[idx], (vec->size - idx) * sizeof(MY_VECTOR_DATA_TYPE));
    }
  } else {
    vec->capacity = MY_VECTOR_RESIZE_POLICIE(vec->size);
    MY_VECTOR_DATA_TYPE* data = NULL;
    MY_CALLOC(data, MY_VECTOR_DATA_TYPE, vec->capacity);

    if (idx != 0) {
      memcpy(data, vec->data, idx * sizeof(MY_VECTOR_DATA_TYPE));
    }
    
    if (idx != vec->size) {
      memcpy(&data[idx + 1], &vec->data[idx], (vec->size - idx) * sizeof(MY_VECTOR_DATA_TYPE));
    }

    MY_FREE(vec->data);
    vec->data = data;
  }

  vec->data[idx] = value;
  vec->size++;
}
void                    MY_VECTOR_FN_PUSH_BACK  (MY_VECTOR_STRUCT* vec, MY_VECTOR_DATA_TYPE value) {
  MY_ASSERT_PTR(vec);

  MY_VECTOR_FN_INSERT(vec, vec->size, value);
}
void                    MY_VECTOR_FN_PUSH_FRONT (MY_VECTOR_STRUCT* vec, MY_VECTOR_DATA_TYPE value) {
  MY_ASSERT_PTR(vec);

  MY_VECTOR_FN_INSERT(vec, 0, value);
}

void                    MY_VECTOR_FN_MEMCPY     (MY_VECTOR_STRUCT* vec, size_t idx, const MY_VECTOR_DATA_TYPE* src, size_t count) {
  MY_ASSERT_PTR(vec);
  MY_ASSERT_PTR(src);

  if (count == 0) { return; }

  MY_ASSERT_BOUNDS(idx, vec->size);
  MY_ASSERT_BOUNDS(idx + count, vec->size + 1);

  memcpy(&vec->data[idx], src, count * sizeof(MY_VECTOR_DATA_TYPE));
}
void                    MY_VECTOR_FN_MEMSET     (MY_VECTOR_STRUCT* vec, size_t idx, MY_VECTOR_DATA_TYPE value, size_t count) {
  MY_ASSERT_PTR(vec);

  if (count == 0) { return; }

  MY_ASSERT_BOUNDS(idx, vec->size);
  MY_ASSERT_BOUNDS(idx + count, vec->size + 1);

  for (size_t i = 0; i < count; i++) {
    vec->data[idx + i] = value;
  }
}

#endif /* MY_VECTOR_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#undef MY_VECTOR_NAME
#undef MY_VECTOR_FN_PREFIX

#undef MY_VECTOR_DATA_TYPE
#undef MY_VECTOR_INITIAL_SIZE
#undef MY_VECTOR_SHRINK_POLICIE
#undef MY_VECTOR_RESIZE_POLICIE
#undef MY_VECTOR_DEALLOCATE_DATA
#undef MY_VECTOR_IMPLEMENTATION

#undef MY_VECTOR_STRUCT

#undef MY_VECTOR_FN_CREATE    
#undef MY_VECTOR_FN_DESTROY

#undef MY_VECTOR_FN_RDLOCK    
#undef MY_VECTOR_FN_WRLOCK    
#undef MY_VECTOR_FN_RDUNLOCK  
#undef MY_VECTOR_FN_WRUNLOCK

#undef MY_VECTOR_FN_DATA      
#undef MY_VECTOR_FN_SIZE      
#undef MY_VECTOR_FN_BACK      
#undef MY_VECTOR_FN_FRONT

#undef MY_VECTOR_FN_SET       
#undef MY_VECTOR_FN_GET       
#undef MY_VECTOR_FN_RESIZE    
#undef MY_VECTOR_FN_SHRINK

#undef MY_VECTOR_FN_CLEAR     
#undef MY_VECTOR_FN_ERASE     
#undef MY_VECTOR_FN_POP_BACK  
#undef MY_VECTOR_FN_POP_FRONT

#undef MY_VECTOR_FN_INSERT    
#undef MY_VECTOR_FN_PUSH_BACK 
#undef MY_VECTOR_FN_PUSH_FRONT

#undef MY_VECTOR_FN_MEMCPY    
#undef MY_VECTOR_FN_MEMSET