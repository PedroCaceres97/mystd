#include <mystd/stdlib.h>

#ifndef MY_SLAB_NAME
    #define MY_SLAB_NAME MySlabInt
#endif /* MY_SLAB_NAME */

#ifndef MY_SLAB_FN_PREFIX
    #define MY_SLAB_FN_PREFIX MY_SLAB_NAME
#endif /* MY_SLAB_FN_PREFIX */

#ifndef MY_SLAB_DATA_TYPE
    #define MY_SLAB_DATA_TYPE int
#endif /* MY_SLAB_DATA_TYPE */

#ifndef MY_SLAB_OBJECTS_COUNT
    #define MY_SLAB_OBJECTS_COUNT 1024
#endif /* MY_SLAB_OBJECTS_COUNT */

/** @cond doxygen_ignore */
#define MY_SLAB_STRUCT          MY_SLAB_NAME
#define MY_SLAB_BLOCK_STRUCT    MY_CONCAT2(MY_SLAB_NAME, Block)

#define MY_SLAB_FN_CREATE       MY_CONCAT2(MY_SLAB_FN_PREFIX, _Create)
#define MY_SLAB_FN_DESTROY      MY_CONCAT2(MY_SLAB_FN_PREFIX, _Destroy)

#define MY_SLAB_FN_RDLOCK       MY_CONCAT2(MY_SLAB_FN_PREFIX, _Rdlock)
#define MY_SLAB_FN_WRLOCK       MY_CONCAT2(MY_SLAB_FN_PREFIX, _Wrlock)
#define MY_SLAB_FN_RDUNLOCK     MY_CONCAT2(MY_SLAB_FN_PREFIX, _Rdunlock)
#define MY_SLAB_FN_WRUNLOCK     MY_CONCAT2(MY_SLAB_FN_PREFIX, _Wrunlock)

#define MY_SLAB_FN_CLEAR        MY_CONCAT2(MY_SLAB_FN_PREFIX, _Clear)
#define MY_SLAB_FN_FREE         MY_CONCAT2(MY_SLAB_FN_PREFIX, _Free)
#define MY_SLAB_FN_ALLOC        MY_CONCAT2(MY_SLAB_FN_PREFIX, _Alloc)

#define MY_SLAB_FN_FOR_EACH     MY_CONCAT2(MY_SLAB_FN_PREFIX, _ForEach)

#define MY_SLAB_FN_SIZE         MY_CONCAT2(MY_SLAB_FN_PREFIX, _Size)
#define MY_SLAB_FN_DUMP         MY_CONCAT2(MY_SLAB_FN_PREFIX, _Dump)
/** @endcond */

#ifdef __cplusplus
extern "C" {
#endif

struct MY_SLAB_STRUCT;
struct MY_SLAB_BLOCK_STRUCT;

typedef struct MY_SLAB_STRUCT MY_SLAB_STRUCT;
typedef struct MY_SLAB_BLOCK_STRUCT MY_SLAB_BLOCK_STRUCT;

MY_SLAB_BLOCK_STRUCT* MY_SLAB_FN_CREATE_BLOCK(MY_SLAB_BLOCK_STRUCT* block);
void                  MY_SLAB_FN_DESTROY_BLOCK(MY_SLAB_BLOCK_STRUCT* block);

MY_SLAB_STRUCT*       MY_SLAB_FN_CREATE(MY_SLAB_STRUCT* slab);
void                  MY_SLAB_FN_DESTROY(MY_SLAB_STRUCT* slab);

void                  MY_SLAB_FN_RDLOCK(MY_SLAB_STRUCT* slab);
void                  MY_SLAB_FN_WRLOCK(MY_SLAB_STRUCT* slab);
void                  MY_SLAB_FN_RDUNLOCK(MY_SLAB_STRUCT* slab);
void                  MY_SLAB_FN_WRUNLOCK(MY_SLAB_STRUCT* slab);

void                  MY_SLAB_FN_CLEAR(MY_SLAB_STRUCT* slab);
void                  MY_SLAB_FN_FREE(MY_SLAB_STRUCT* slab, MY_SLAB_DATA_TYPE* ptr);
MY_SLAB_DATA_TYPE*    MY_SLAB_FN_ALLOC(MY_SLAB_STRUCT* slab);

void                  MY_SLAB_FN_FOR_EACH(MY_SLAB_STRUCT* slab, void* user, bool (*each)(MY_SLAB_DATA_TYPE*, void*));

size_t                MY_SLAB_FN_SIZE(MY_SLAB_STRUCT* slab);
void                  MY_SLAB_FN_DUMP(MY_SLAB_STRUCT* slab, MyFile file);

struct MY_SLAB_BLOCK_STRUCT {
    int                             allocated;
    MY_SLAB_DATA_TYPE               mem[MY_SLAB_OBJECTS_COUNT];
    size_t                          free_count;
    MY_SLAB_DATA_TYPE*              free_list[MY_SLAB_OBJECTS_COUNT];
    bool                            allocated_list[MY_SLAB_OBJECTS_COUNT];
    struct MY_SLAB_BLOCK_STRUCT*    next;
};

struct MY_SLAB_STRUCT {
    int                   allocated;
    MY_SLAB_BLOCK_STRUCT  block;
    size_t                total;
    MY_RWLOCK_TYPE        lock;
};

#ifdef MY_SLAB_IMPLEMENTATION

MY_SLAB_BLOCK_STRUCT* MY_SLAB_FN_CREATE_BLOCK(MY_SLAB_BLOCK_STRUCT* block) {
    if (!block) {
        MY_CALLOC(block, MY_SLAB_BLOCK_STRUCT, 1);
        block->allocated = true;
    } else {
        block->allocated = false;
    }

    block->free_count = MY_SLAB_OBJECTS_COUNT;
    for (size_t i = 0; i < MY_SLAB_OBJECTS_COUNT; i++) {
        block->free_list[i] = &block->mem[MY_SLAB_OBJECTS_COUNT - i - 1];
    }

    return block;
}
void MY_SLAB_FN_DESTROY_BLOCK(MY_SLAB_BLOCK_STRUCT* block) {
    MY_ASSERT_PTR(block);

    if (block->allocated) {
        MY_FREE(block);
    }
}

MY_SLAB_STRUCT* MY_SLAB_FN_CREATE(MY_SLAB_STRUCT* slab) {
    if (!slab) {
        MY_CALLOC(slab, MY_SLAB_STRUCT, 1);
        slab->allocated = true;
    } else {
        slab->allocated = false;
    }

    MY_RWLOCK_INIT(slab->lock);
    MY_SLAB_FN_CREATE_BLOCK(&slab->block);

    return slab;
}
void MY_SLAB_FN_DESTROY(MY_SLAB_STRUCT* slab) {
    MY_ASSERT_PTR(slab);
    MY_ASSERT(slab->total == 0, "Trying to free a non-empty slab");

    MY_SLAB_BLOCK_STRUCT* current = &slab->block;
    while (current) {
        MY_SLAB_BLOCK_STRUCT* next = current->next;
        MY_SLAB_FN_DESTROY_BLOCK(current);
        current = next;
    }
}

void MY_SLAB_FN_RDLOCK(MY_SLAB_STRUCT* slab) {
    MY_ASSERT_PTR(slab);
    MY_RWLOCK_RDLOCK(slab->lock);
}
void MY_SLAB_FN_WRLOCK(MY_SLAB_STRUCT* slab) {
    MY_ASSERT_PTR(slab);
    MY_RWLOCK_WRLOCK(slab->lock);
}
void MY_SLAB_FN_RDUNLOCK(MY_SLAB_STRUCT* slab) {
    MY_ASSERT_PTR(slab);
    MY_RWLOCK_RDUNLOCK(slab->lock);
}
void MY_SLAB_FN_WRUNLOCK(MY_SLAB_STRUCT* slab) {
    MY_ASSERT_PTR(slab);
    MY_RWLOCK_WRUNLOCK(slab->lock);
}

void MY_SLAB_FN_CLEAR(MY_SLAB_STRUCT* slab) {
    MY_ASSERT_PTR(slab);

    MY_SLAB_BLOCK_STRUCT* current = &slab->block;
    while (current) {
        memset(current->allocated_list, 0, MY_SLAB_OBJECTS_COUNT);
        memset(current->mem, 0, MY_SLAB_OBJECTS_COUNT * sizeof(MY_SLAB_DATA_TYPE));
        current->free_count = MY_SLAB_OBJECTS_COUNT;
        for (size_t i = 0; i < MY_SLAB_OBJECTS_COUNT; i++) {
            current->free_list[i] = &current->mem[MY_SLAB_OBJECTS_COUNT - i - 1];
        }

        MY_SLAB_BLOCK_STRUCT* next = current->next;
        MY_SLAB_FN_DESTROY_BLOCK(current);
        current = next;
    }
}
void MY_SLAB_FN_FREE(MY_SLAB_STRUCT* slab, MY_SLAB_DATA_TYPE* ptr) {
    MY_ASSERT_PTR(slab);
    MY_ASSERT_PTR(ptr);

    MY_SLAB_BLOCK_STRUCT* current = &slab->block;
    while (current) {
        if (ptr >= &current->mem[0] && ptr <= &current->mem[MY_SLAB_OBJECTS_COUNT - 1]) {
            MY_ASSERT(current->allocated_list[(size_t)(ptr - current->mem)], "Ptr belongs to slab but was not allocated");

            current->allocated_list[(size_t)(ptr - current->mem)] = false;
            current->free_list[current->free_count] = ptr;
            current->free_count++;
            slab->total--;
            return;
        }

        current = current->next;
    }

    MY_ASSERT(false, "Trying to free a foreign ptr");
}
MY_SLAB_DATA_TYPE* MY_SLAB_FN_ALLOC(MY_SLAB_STRUCT* slab) {
    MY_ASSERT_PTR(slab);

    MY_SLAB_BLOCK_STRUCT* current = &slab->block;
    while (current) {
        if (current->free_count > 0) {
            slab->total++;
            current->free_count--;
            MY_SLAB_DATA_TYPE* ptr = current->free_list[current->free_count];
            current->allocated_list[(size_t)(ptr - current->mem)] = true;
            return ptr;
        }

        if (current->next == NULL) {
            current->next = MY_SLAB_FN_CREATE_BLOCK(NULL);
        }
        current = current->next;
    }

    return NULL; /* Never reaching here */
}
void MY_SLAB_FN_FOR_EACH(MY_SLAB_STRUCT* slab, void* user, bool (*each)(MY_SLAB_DATA_TYPE*, void*)) {
    MY_ASSERT_PTR(slab);
    MY_ASSERT_PTR(each);

    MY_SLAB_BLOCK_STRUCT* current = &slab->block;
    while (current) {
        for (int i = 0; i < MY_SLAB_OBJECTS_COUNT; i++) {
            if (!current->allocated_list[i]) {
                continue;
            }

            if (each(&current->mem[i], user)) {
                return;
            }
        }

        current = current->next;
    }
}

size_t MY_SLAB_FN_SIZE(MY_SLAB_STRUCT* slab) {
    MY_ASSERT_PTR(slab);
    return slab->total;
}
void MY_SLAB_FN_DUMP(MY_SLAB_STRUCT* slab, MyFile* file) {
    MY_ASSERT_PTR(slab);
    MyFprintf(
        file, 
        "Slab Information:\n"
        " Allocated %s('s): %zu\n"
        " Total bytes allocated: %zu\n\n", 
        MY_STR(MY_SLAB_DATA_TYPE), slab->total, 
        slab->total * sizeof(MY_SLAB_DATA_TYPE));
}

#endif /* MY_SLAB_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#undef MY_SLAB_NAME
#undef MY_SLAB_FN_PREFIX

#undef MY_SLAB_DATA_TYPE
#undef MY_SLAB_OBJECTS_COUNT

#undef MY_SLAB_STRUCT
#undef MY_SLAB_BLOCK_STRUCT

#undef MY_SLAB_FN_CREATE
#undef MY_SLAB_FN_DESTROY

#undef MY_SLAB_FN_RDLOCK
#undef MY_SLAB_FN_WRLOCK
#undef MY_SLAB_FN_RDUNLOCK
#undef MY_SLAB_FN_WRUNLOCK

#undef MY_SLAB_FN_CLEAR
#undef MY_SLAB_FN_FREE
#undef MY_SLAB_FN_ALLOC

#undef MY_SLAB_FN_FOR_EACH

#undef MY_SLAB_FN_SIZE
#undef MY_SLAB_FN_PRINT_OUT
#undef MY_SLAB_FN_PRINT_ERR