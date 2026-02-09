#ifndef __MYSTD_TRACKER_H__
#define __MYSTD_TRACKER_H__ 

#include <mystd/stdio.h>

#ifndef MY_TRACKER_NAME
    #define MY_TRACKER_NAME MyTracker
#endif /* MY_TRACKER_NAME */

#ifndef MY_TRACKER_FN_PREFIX
    #define MY_TRACKER_FN_PREFIX      MY_TRACKER_NAME
#endif /* MY_TRACKER_FN_PREFIX */

#define MY_TRACKER_STRUCT           MY_TRACKER_NAME
#define MY_TRACKER_PTRHDR_STRUCT    MY_CONCAT2(MY_TRACKER_STRUCT, Ptrhdr)
#define MY_TRACKER_LIST_STRUCT      MY_CONCAT2(MY_TRACKER_STRUCT, List)
#define MY_TRACKER_NODE_STRUCT      MY_CONCAT2(MY_TRACKER_STRUCT, Node)

#define MY_TRACKER_FN_CREATE        MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Create)
#define MY_TRACKER_FN_DESTROY       MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Destroy)

#define MY_TRACKER_FN_RDLOCK        MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Rdlock)
#define MY_TRACKER_FN_WRLOCK        MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Wrlock)
#define MY_TRACKER_FN_RDUNLOCK      MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Rdunlock)
#define MY_TRACKER_FN_WRUNLOCK      MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Wrunlock)

#define MY_TRACKER_FN_CLEAR         MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Clear)
#define MY_TRACKER_FN_FREE          MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Free)
#define MY_TRACKER_FN_ALLOC         MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Alloc)
#define MY_TRACKER_FN_REALLOC       MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Realloc)
#define MY_TRACKER_FN_DUPLICATE     MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Duplicate)

#define MY_TRACKER_FN_BYTES         MY_CONCAT2(MY_TRACKER_FN_PREFIX, _bytes)
#define MY_TRACKER_FN_COUNT         MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Count)
#define MY_TRACKER_FN_DUMP          MY_CONCAT2(MY_TRACKER_FN_PREFIX, _Dump)

#ifdef __cplusplus
extern "C" {
#endif

struct MY_TRACKER_STRUCT;
struct MY_TRACKER_PTRHDR_STRUCT;
struct MY_TRACKER_NODE_STRUCT;
struct MY_TRACKER_LIST_STRUCT;

typedef struct MY_TRACKER_PTRHDR_STRUCT MY_TRACKER_PTRHDR_STRUCT;
typedef struct MY_TRACKER_STRUCT MY_TRACKER_STRUCT;
typedef struct MY_TRACKER_NODE_STRUCT MY_TRACKER_NODE_STRUCT;
typedef struct MY_TRACKER_LIST_STRUCT MY_TRACKER_LIST_STRUCT;

MY_TRACKER_STRUCT*  MY_TRACKER_FN_CREATE    (MY_TRACKER_STRUCT* tracker, MyContext context);
void                MY_TRACKER_FN_DESTROY   (MY_TRACKER_STRUCT* tracker);

void                MY_TRACKER_FN_RDLOCK    (MY_TRACKER_STRUCT* tracker);
void                MY_TRACKER_FN_WRLOCK    (MY_TRACKER_STRUCT* tracker);
void                MY_TRACKER_FN_RDUNLOCK  (MY_TRACKER_STRUCT* tracker);
void                MY_TRACKER_FN_WRUNLOCK  (MY_TRACKER_STRUCT* tracker);

void                MY_TRACKER_FN_CLEAR     (MY_TRACKER_STRUCT* tracker);
void                MY_TRACKER_FN_FREE      (MY_TRACKER_STRUCT* tracker, void* ptr);
void*               MY_TRACKER_FN_ALLOC     (MY_TRACKER_STRUCT* tracker, MyContext context, size_t size);
void*               MY_TRACKER_FN_REALLOC   (MY_TRACKER_STRUCT* tracker, MyContext context, void* ptr, size_t size);
void*               MY_TRACKER_FN_DUPLICATE (MY_TRACKER_STRUCT* tracker, MyContext context, void* ptr, size_t size);

size_t              MY_TRACKER_FN_BYTES     (MY_TRACKER_STRUCT* tracker);
size_t              MY_TRACKER_FN_COUNT     (MY_TRACKER_STRUCT* tracker);
void                MY_TRACKER_FN_DUMP      (MY_TRACKER_STRUCT* tracker, MyFile* file);

struct MY_TRACKER_NODE_STRUCT {
    MY_TRACKER_PTRHDR_STRUCT*       data;
    struct MY_TRACKER_NODE_STRUCT*  next;
    struct MY_TRACKER_NODE_STRUCT*  prev;
};
struct MY_TRACKER_LIST_STRUCT {
    size_t                          size;
    struct MY_TRACKER_NODE_STRUCT*  back;
    struct MY_TRACKER_NODE_STRUCT*  front;
};

struct MY_TRACKER_PTRHDR_STRUCT {
    MY_TRACKER_STRUCT*        tracker;
    MY_TRACKER_NODE_STRUCT    node;
    size_t                    size;
    MyContext                 context;
};
struct MY_TRACKER_STRUCT {
    MY_RWLOCK_TYPE            lock;
    MY_TRACKER_LIST_STRUCT    ptrs;
    size_t                    bytes;
    MyContext                 context;
    int                       allocated;
};

#ifdef MY_TRACKER_IMPLEMENTATION

#define MY_TRACKER_FREE(ptr)                   do { free((void*)(ptr)); (ptr) = NULL;                                                                       } while(0)
#define MY_TRACKER_MALLOC(v, type, size)       do { (v) = (type*)malloc((size));                MY_ASSERT_MALLOC((v), type, (size)); memset(v, 0, (size));  } while(0)
#define MY_TRACKER_CALLOC(v, type, count)      do { (v) = (type*)calloc((count), sizeof(type)); MY_ASSERT_CALLOC((v), type, (count));                       } while(0)

#define MY_TRACKER_PTR_TO_HDR(ptr) ((MY_TRACKER_PTRHDR_STRUCT*)  MY_PTR_SUB(ptr, sizeof(struct MY_TRACKER_PTRHDR_STRUCT)))
#define MY_TRACKER_HDR_TO_PTR(hdr) ((void*)                      MY_PTR_ADD(hdr, sizeof(struct MY_TRACKER_PTRHDR_STRUCT)))

static MY_TRACKER_NODE_STRUCT* MY_TRACKER_LIST_FN_INSERT(MY_TRACKER_LIST_STRUCT* list, MY_TRACKER_NODE_STRUCT* pivot, MY_TRACKER_NODE_STRUCT* node) {
    node->prev = pivot;

    if (pivot->next) {
        node->next = pivot->next;
        pivot->next->prev = node;
    } else {
        node->next = NULL;
        list->back = node;
    }

    pivot->next = node;
    list->size++;
}
static void MY_TRACKER_LIST_FN_ERASE(MY_TRACKER_LIST_STRUCT* list, MY_TRACKER_NODE_STRUCT* node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        list->front = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        list->back = node->prev;
    }

    list->size--;
}

MY_TRACKER_STRUCT*  MY_TRACKER_FN_CREATE(MY_TRACKER_STRUCT* tracker, MyContext context) {
    if (!tracker) {
        MY_CALLOC(tracker, struct MY_TRACKER_STRUCT, 1);
        tracker->allocated = true;
    } else {
        tracker->allocated = false;
    }

    MY_RWLOCK_INIT(tracker->lock);

    tracker->bytes   = 0;
    tracker->context = context;
    tracker->ptrs.size = 0;
    tracker->ptrs.back = NULL;
    tracker->ptrs.front = NULL;

    return tracker;
}
void                MY_TRACKER_FN_DESTROY(MY_TRACKER_STRUCT* tracker) {
    MY_ASSERT_PTR(tracker);
    MY_ASSERT(tracker->ptrs.size == 0, "Destroying non empty tracker (HINT: Call free all)");

    if (tracker->allocated) {
        MY_TRACKER_FREE(tracker);
    }
}

void                MY_TRACKER_FN_RDLOCK(MY_TRACKER_STRUCT* tracker) {
  MY_ASSERT_PTR(tracker);
  MY_RWLOCK_RDLOCK(tracker->lock);
}
void                MY_TRACKER_FN_WRLOCK(MY_TRACKER_STRUCT* tracker) {
  MY_ASSERT_PTR(tracker);
  MY_RWLOCK_WRLOCK(tracker->lock);
}
void                MY_TRACKER_FN_RDUNLOCK(MY_TRACKER_STRUCT* tracker) {
  MY_ASSERT_PTR(tracker);
  MY_RWLOCK_RDUNLOCK(tracker->lock);
}
void                MY_TRACKER_FN_WRUNLOCK(MY_TRACKER_STRUCT* tracker) {
  MY_ASSERT_PTR(tracker);
  MY_RWLOCK_WRUNLOCK(tracker->lock);
}

void                MY_TRACKER_FN_CLEAR(MY_TRACKER_STRUCT* tracker) {
  MY_ASSERT_PTR(tracker);

  MY_TRACKER_NODE_STRUCT* current = tracker->ptrs.front;
  while (current != NULL) {
    MY_TRACKER_PTRHDR_STRUCT* hdr = current->data;
    current = current->next;
    MY_TRACKER_LIST_FN_ERASE(&tracker->ptrs, tracker->ptrs.front);
    tracker->bytes -= hdr->size;
    MY_TRACKER_FREE(hdr);
  }
}
void                MY_TRACKER_FN_FREE(MY_TRACKER_STRUCT* tracker, void* ptr) {
  MY_ASSERT_PTR(tracker);
  MY_ASSERT_PTR(ptr);

  MY_TRACKER_PTRHDR_STRUCT* hdr = MY_TRACKER_PTR_TO_HDR(ptr);
  MY_ASSERT(hdr->tracker == tracker, "Freeing a foreign ptr");

  MY_TRACKER_LIST_FN_ERASE(&tracker->ptrs, &hdr->node);
  tracker->bytes -= hdr->size;

  MY_TRACKER_FREE(hdr);
}
void*               MY_TRACKER_FN_ALLOC(MY_TRACKER_STRUCT* tracker, MyContext context, size_t size) {
  MY_ASSERT_PTR(tracker);
  MY_ASSERT(size != 0, "Requested allocation size is 0");

  MY_TRACKER_PTRHDR_STRUCT* hdr = NULL;
  MY_TRACKER_MALLOC(hdr, struct MY_TRACKER_PTRHDR_STRUCT, size + sizeof(struct MY_TRACKER_PTRHDR_STRUCT));

  MY_TRACKER_LIST_FN_INSERT(&tracker->ptrs, tracker->ptrs.back, &hdr->node);
  hdr->node.data = hdr;
  hdr->tracker = tracker;
  hdr->size = size;
  hdr->context = context;

  tracker->bytes += size;
  void* ptr = MY_TRACKER_HDR_TO_PTR(hdr);
  return ptr;
}
void*               MY_TRACKER_FN_REALLOC(MY_TRACKER_STRUCT* tracker, MyContext context, void* ptr, size_t size) {
  MY_ASSERT_PTR(tracker);
  MY_ASSERT_PTR(ptr);
  MY_ASSERT(size != 0, "Requested allocation size is 0");

  MY_TRACKER_PTRHDR_STRUCT* srchdr = MY_TRACKER_PTR_TO_HDR(ptr);
  MY_ASSERT(srchdr->tracker == tracker, "Reallocating a foreign ptr");

  MY_TRACKER_PTRHDR_STRUCT* newhdr = NULL;
  MY_TRACKER_MALLOC(newhdr, struct MY_TRACKER_PTRHDR_STRUCT, size + sizeof(struct MY_TRACKER_PTRHDR_STRUCT));

  MY_TRACKER_LIST_FN_INSERT(&tracker->ptrs, &srchdr->node, &newhdr->node);
  MY_TRACKER_LIST_FN_ERASE(&tracker->ptrs, &srchdr->node);
  newhdr->node.data = newhdr;
  newhdr->tracker = tracker;
  newhdr->size = size;
  newhdr->context = context;

  tracker->bytes -= srchdr->size;
  tracker->bytes += size;
  void*  newptr = MY_TRACKER_HDR_TO_PTR(newhdr);
  memcpy(newptr, ptr, MY_MIN(size, srchdr->size));
  MY_TRACKER_FREE(srchdr);

  return newptr;
}
void*               MY_TRACKER_FN_DUPLICATE(MY_TRACKER_STRUCT* tracker, MyContext context, void* ptr, size_t size) {
  MY_ASSERT_PTR(tracker);
  MY_ASSERT_PTR(ptr);
  MY_ASSERT(size != 0, "Requested allocation size is 0");

  MY_TRACKER_PTRHDR_STRUCT* srchdr = MY_TRACKER_PTR_TO_HDR(ptr);
  MY_ASSERT(srchdr->tracker == tracker, "Duplicating a foreign ptr");

  MY_TRACKER_PTRHDR_STRUCT* newhdr = NULL;
  MY_TRACKER_MALLOC(newhdr, struct MY_TRACKER_PTRHDR_STRUCT, size + sizeof(struct MY_TRACKER_PTRHDR_STRUCT));

  MY_TRACKER_LIST_FN_INSERT(&tracker->ptrs, &srchdr->node, &newhdr->node);
  newhdr->node.data = newhdr;
  newhdr->tracker = tracker;
  newhdr->size = size;
  newhdr->context = context;

  tracker->bytes += size;
  void*  newptr = MY_TRACKER_HDR_TO_PTR(newhdr);
  memcpy(newptr, ptr, MY_MIN(size, srchdr->size));
  return newptr;
}

size_t              MY_TRACKER_FN_BYTES(MY_TRACKER_STRUCT* tracker) {
  MY_ASSERT_PTR(tracker);
  return tracker->bytes;
}
size_t              MY_TRACKER_FN_COUNT(MY_TRACKER_STRUCT* tracker) {
  MY_ASSERT_PTR(tracker);
  return tracker->ptrs.size;
}
void                MY_TRACKER_FN_DUMP(MY_TRACKER_STRUCT* tracker, MyFile* file) {
    MY_ASSERT_PTR(tracker);

    MyFprintf( 
        file,
        "MY_TRACKER Information:\n"
        " Alias: %s\n"
        " Origin context: %s:%u (%s)\n"
        " Allocated pointers: %zu\n"
        " Total bytes allocated: %zu\n\n",
        tracker->context.alias,
        tracker->context.file,
        tracker->context.line,
        tracker->context.func,
        tracker->ptrs.size,
        tracker->bytes
    );

    if (tracker->bytes == 0) {
        return;
    }

    MY_TRACKER_NODE_STRUCT* current = tracker->ptrs.front;
    MyFprintf(file, "%s", "Individual Pointer Information\n");
    size_t index = 0;
    while (current != NULL) {
        MY_TRACKER_PTRHDR_STRUCT* hdr = current->data;
        
        MyFprintf( 
            file,
            " Pointer [%zu]\n"
            "  Size: %zu bytes\n"
            "  Alias: %s\n"
            "  Addres: %p\n"
            "  Origin context: %s:%u (%s)\n\n",
            index, 
            hdr->size, 
            hdr->context.alias,
            MY_TRACKER_HDR_TO_PTR(hdr), 
            hdr->context.file, 
            hdr->context.line, 
            hdr->context.func
        );
      
        current = current->next;
        index++;
    }
}

#endif /* MY_TRACKER_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* __MYSTD_MY_TRACKER_H__ */