#include <mystd/stdlib.h>

#ifndef MY_LIST_NAME
  #define MY_LIST_NAME MyListInt
#endif /* MY_LIST_NAME */

#ifndef MY_LIST_FN_PREFIX
  #define MY_LIST_FN_PREFIX MY_LIST_NAME
#endif /* MY_LIST_FN_PREFIX */

#ifndef MY_LIST_DATA_TYPE
  #define MY_LIST_DATA_TYPE int
#endif /* MY_LIST_DATA_TYPE */

/** @cond doxygen_ignore */
#define MY_LIST_STRUCT              MY_LIST_NAME
#define MY_LIST_NODE_STRUCT         MY_CONCAT2(MY_LIST_NAME, Node)

#define MY_LIST_FN_CREATE           MY_CONCAT2(MY_LIST_FN_PREFIX, _Create)
#define MY_LIST_FN_DESTROY          MY_CONCAT2(MY_LIST_FN_PREFIX, _Destroy)

#define MY_LIST_FN_RDLOCK           MY_CONCAT2(MY_LIST_FN_PREFIX, _Rdlock)
#define MY_LIST_FN_WRLOCK           MY_CONCAT2(MY_LIST_FN_PREFIX, _Wrlock)
#define MY_LIST_FN_RDUNLOCK         MY_CONCAT2(MY_LIST_FN_PREFIX, _Rdunlock)
#define MY_LIST_FN_WRUNLOCK         MY_CONCAT2(MY_LIST_FN_PREFIX, _Wrunlock)

#define MY_LIST_FN_GET              MY_CONCAT2(MY_LIST_FN_PREFIX, _Get)
#define MY_LIST_FN_SIZE             MY_CONCAT2(MY_LIST_FN_PREFIX, _Size)
#define MY_LIST_FN_BACK             MY_CONCAT2(MY_LIST_FN_PREFIX, _Back)
#define MY_LIST_FN_FRONT            MY_CONCAT2(MY_LIST_FN_PREFIX, _Front)

#define MY_LIST_FN_CLEAR            MY_CONCAT2(MY_LIST_FN_PREFIX, _Clear)
#define MY_LIST_FN_ERASE            MY_CONCAT2(MY_LIST_FN_PREFIX, _Erase)
#define MY_LIST_FN_POP_BACK         MY_CONCAT2(MY_LIST_FN_PREFIX, _PopBack)
#define MY_LIST_FN_POP_FRONT        MY_CONCAT2(MY_LIST_FN_PREFIX, _PopFront)

#define MY_LIST_FN_INSERT_NEXT      MY_CONCAT2(MY_LIST_FN_PREFIX, _InsertNext)
#define MY_LIST_FN_INSERT_PREV      MY_CONCAT2(MY_LIST_FN_PREFIX, _InsertPrev)
#define MY_LIST_FN_PUSH_BACK        MY_CONCAT2(MY_LIST_FN_PREFIX, _PushBack)
#define MY_LIST_FN_PUSH_FRONT       MY_CONCAT2(MY_LIST_FN_PREFIX, _PushFront)

#define MY_LIST_NODE_FN_CREATE      MY_CONCAT2(MY_LIST_FN_PREFIX, Node_Create)
#define MY_LIST_NODE_FN_DUPLICATE   MY_CONCAT2(MY_LIST_FN_PREFIX, Node_Duplicate)
#define MY_LIST_NODE_FN_DESTROY     MY_CONCAT2(MY_LIST_FN_PREFIX, Node_Destroy)

#define MY_LIST_NODE_FN_SET         MY_CONCAT2(MY_LIST_FN_PREFIX, Node_Set)
#define MY_LIST_NODE_FN_GET         MY_CONCAT2(MY_LIST_FN_PREFIX, Node_Get)
#define MY_LIST_NODE_FN_NEXT        MY_CONCAT2(MY_LIST_FN_PREFIX, Node_Next)
#define MY_LIST_NODE_FN_PREV        MY_CONCAT2(MY_LIST_FN_PREFIX, Node_Prev)
#define MY_LIST_NODE_FN_LIST        MY_CONCAT2(MY_LIST_FN_PREFIX, Node_List)
/** @endcond */

#ifdef __cplusplus
extern "C" {
#endif

struct MY_LIST_NODE_STRUCT;
struct MY_LIST_STRUCT;

typedef struct MY_LIST_NODE_STRUCT MY_LIST_NODE_STRUCT;
typedef struct MY_LIST_STRUCT MY_LIST_STRUCT;

MY_LIST_STRUCT*         MY_LIST_FN_CREATE           (MY_LIST_STRUCT* list);
void                    MY_LIST_FN_DESTROY          (MY_LIST_STRUCT* list);

void                    MY_LIST_FN_RDLOCK           (MY_LIST_STRUCT* list);
void                    MY_LIST_FN_WRLOCK           (MY_LIST_STRUCT* list);
void                    MY_LIST_FN_RDUNLOCK         (MY_LIST_STRUCT* list);
void                    MY_LIST_FN_WRUNLOCK         (MY_LIST_STRUCT* list);

MY_LIST_NODE_STRUCT*    MY_LIST_FN_GET              (MY_LIST_STRUCT* list, size_t index);
size_t                  MY_LIST_FN_SIZE             (MY_LIST_STRUCT* list);
MY_LIST_NODE_STRUCT*    MY_LIST_FN_BACK             (MY_LIST_STRUCT* list);
MY_LIST_NODE_STRUCT*    MY_LIST_FN_FRONT            (MY_LIST_STRUCT* list);

void                    MY_LIST_FN_CLEAR            (MY_LIST_STRUCT* list, int deallocate);
void                    MY_LIST_FN_ERASE            (MY_LIST_STRUCT* list, MY_LIST_NODE_STRUCT* node, int deallocate);
void                    MY_LIST_FN_POP_BACK         (MY_LIST_STRUCT* list, int deallocate);
void                    MY_LIST_FN_POP_FRONT        (MY_LIST_STRUCT* list, int deallocate);

void                    MY_LIST_FN_INSERT_NEXT      (MY_LIST_STRUCT* list, MY_LIST_NODE_STRUCT* pivot, MY_LIST_NODE_STRUCT* node);
void                    MY_LIST_FN_INSERT_PREV      (MY_LIST_STRUCT* list, MY_LIST_NODE_STRUCT* pivot, MY_LIST_NODE_STRUCT* node);
void                    MY_LIST_FN_PUSH_BACK        (MY_LIST_STRUCT* list, MY_LIST_NODE_STRUCT* node);
void                    MY_LIST_FN_PUSH_FRONT       (MY_LIST_STRUCT* list, MY_LIST_NODE_STRUCT* node);

MY_LIST_NODE_STRUCT*    MY_LIST_NODE_FN_CREATE      (MY_LIST_NODE_STRUCT* node);
MY_LIST_NODE_STRUCT*    MY_LIST_NODE_FN_DUPLICATE   (MY_LIST_NODE_STRUCT* src, MY_LIST_NODE_STRUCT* dst);
void                    MY_LIST_NODE_FN_DESTROY     (MY_LIST_NODE_STRUCT* node);

void                    MY_LIST_NODE_FN_SET         (MY_LIST_NODE_STRUCT* node, MY_LIST_DATA_TYPE value);
MY_LIST_DATA_TYPE       MY_LIST_NODE_FN_GET         (MY_LIST_NODE_STRUCT* node);
MY_LIST_NODE_STRUCT*    MY_LIST_NODE_FN_NEXT        (MY_LIST_NODE_STRUCT* node);
MY_LIST_NODE_STRUCT*    MY_LIST_NODE_FN_PREV        (MY_LIST_NODE_STRUCT* node);
MY_LIST_STRUCT*         MY_LIST_NODE_FN_LIST        (MY_LIST_NODE_STRUCT* node);

struct MY_LIST_NODE_STRUCT {
    int                     allocated;
    MY_LIST_DATA_TYPE       data;
    MY_LIST_NODE_STRUCT*    next;
    MY_LIST_NODE_STRUCT*    prev;
    MY_LIST_STRUCT*         list;
};

struct MY_LIST_STRUCT {
    int                     allocated;
    size_t                  size;
    MY_LIST_NODE_STRUCT*    front;
    MY_LIST_NODE_STRUCT*    back;
    MY_RWLOCK_TYPE          lock;
};

#ifdef MY_LIST_IMPLEMENTATION

MY_LIST_STRUCT* MY_LIST_FN_CREATE(MY_LIST_STRUCT* list) {
    if (!list) {
        MY_CALLOC(list, MY_LIST_STRUCT, 1);
        list->allocated = true;
    } else {
        list->allocated = false;
    }

    list->size = 0;
    list->front = NULL;
    list->back = NULL;
    MY_RWLOCK_INIT(list->lock);

    return list;
}
void MY_LIST_FN_DESTROY(MY_LIST_STRUCT* list) {
    MY_ASSERT_PTR(list);
    MY_ASSERT(list->size == 0, "Destroying non-empty list (HINT: Clear the list)");

    MY_RWLOCK_DESTROY(list->lock);

    if (list->allocated) {
        MY_FREE(list);
    }
}

void MY_LIST_FN_RDLOCK(MY_LIST_STRUCT* list) {
    MY_ASSERT_PTR(list);
    MY_RWLOCK_RDLOCK(list->lock);
}
void MY_LIST_FN_WRLOCK(MY_LIST_STRUCT* list) {
    MY_ASSERT_PTR(list);
    MY_RWLOCK_WRLOCK(list->lock);
}
void MY_LIST_FN_RDUNLOCK(MY_LIST_STRUCT* list) {
    MY_ASSERT_PTR(list);
    MY_RWLOCK_RDUNLOCK(list->lock);
}
void MY_LIST_FN_WRUNLOCK(MY_LIST_STRUCT* list) {
    MY_ASSERT_PTR(list);
    MY_RWLOCK_WRUNLOCK(list->lock);
}

MY_LIST_NODE_STRUCT* MY_LIST_FN_GET(MY_LIST_STRUCT* list, size_t idx) {
    MY_ASSERT_PTR(list);
    MY_ASSERT_BOUNDS(idx, list->size);

    MY_LIST_NODE_STRUCT* current = NULL;
    if (idx < list->size / 2) {
        current = list->front;
        for (size_t i = 0; i < idx; i++) {
            current = current->next;
        }
    } else {
        current = list->back;
        for (size_t i = list->size - 1; i > idx; i--) {
            current = current->prev;
        }
    }
    return current;
}
size_t MY_LIST_FN_SIZE(MY_LIST_STRUCT* list) {
    MY_ASSERT_PTR(list);
    return list->size;
}
MY_LIST_NODE_STRUCT* MY_LIST_FN_BACK(MY_LIST_STRUCT* list) {
    MY_ASSERT_PTR(list);
    return list->back;
}
MY_LIST_NODE_STRUCT* MY_LIST_FN_FRONT(MY_LIST_STRUCT* list) {
    MY_ASSERT_PTR(list);
    return list->front;
}

void MY_LIST_FN_CLEAR(MY_LIST_STRUCT* list, int deallocate) {
    MY_ASSERT_PTR(list);

    if (list->size == 0) {
        return;
    }

    MY_LIST_NODE_STRUCT* next = NULL;
    MY_LIST_NODE_STRUCT* current = list->front;
    while (current) {
        next = current->next;
        if (deallocate) {
            MY_LIST_NODE_FN_DESTROY(current);
        } else {
            current->list = NULL;
            current->next = NULL;
            current->prev = NULL;
        }
        current = next;
    }

    list->size = 0;
    list->front = NULL;
    list->back = NULL;
}
void MY_LIST_FN_ERASE(MY_LIST_STRUCT* list, MY_LIST_NODE_STRUCT* node, int deallocate) {
    MY_ASSERT_PTR(list);
    MY_ASSERT_PTR(node);
    MY_ASSERT(node->list == list, "Erasing a foreign node");

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

    if (deallocate) {
        MY_LIST_NODE_FN_DESTROY(node);
    } else {
        node->list = NULL;
        node->next = NULL;
        node->prev = NULL;
    }
}
void MY_LIST_FN_POP_BACK(MY_LIST_STRUCT* list, int deallocate) {
    MY_ASSERT_PTR(list);

    if (list->size == 0) {
        MY_EMPTY_POPPING();
        return;
    }

    MY_LIST_FN_ERASE(list, list->back, deallocate);
}
void MY_LIST_FN_POP_FRONT(MY_LIST_STRUCT* list, int deallocate) {
    MY_ASSERT_PTR(list);

    if (list->size == 0) {
        MY_EMPTY_POPPING();
        return;
    }

    MY_LIST_FN_ERASE(list, list->front, deallocate);
}

void MY_LIST_FN_INSERT_NEXT(MY_LIST_STRUCT* list, MY_LIST_NODE_STRUCT* pivot, MY_LIST_NODE_STRUCT* node) {
    MY_ASSERT_PTR(list);
    MY_ASSERT_PTR(pivot);
    MY_ASSERT_PTR(node);
    MY_ASSERT(node->list == NULL, "Inserting a foreign node (HINT: Duplicate the node)");
    MY_ASSERT(pivot->list == list, "The pivot is a foreign node");

    node->list = list;
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
void MY_LIST_FN_INSERT_PREV(MY_LIST_STRUCT* list, MY_LIST_NODE_STRUCT* pivot, MY_LIST_NODE_STRUCT* node) {
    MY_ASSERT_PTR(list);
    MY_ASSERT_PTR(pivot);
    MY_ASSERT_PTR(node);
    MY_ASSERT(node->list == NULL, "Inserting a foreign node (HINT: Duplicate the node)");
    MY_ASSERT(pivot->list == list, "The pivot is a foreign node");

    node->list = list;
    node->next = pivot;

    if (pivot->prev) {
        node->prev = pivot->prev;
        pivot->prev->next = node;
    } else {
        node->prev = NULL;
        list->front = node;
    }

    pivot->prev = node;
    list->size++;
}
void MY_LIST_FN_PUSH_BACK(MY_LIST_STRUCT* list, MY_LIST_NODE_STRUCT* node) {
    MY_ASSERT_PTR(list);
    MY_ASSERT_PTR(node);
    MY_ASSERT(node->list == NULL, "Pushing a foreign node (HINT: Duplicate the node)");

    MY_LIST_FN_INSERT_NEXT(list, list->back, node);
}
void MY_LIST_FN_PUSH_FRONT(MY_LIST_STRUCT* list, MY_LIST_NODE_STRUCT* node) {
    MY_ASSERT_PTR(list);
    MY_ASSERT_PTR(node);
    MY_ASSERT(node->list == NULL, "Pushing a foreign node (HINT: Duplicate the node)");

    MY_LIST_FN_INSERT_PREV(list, list->front, node);
}

MY_LIST_NODE_STRUCT* MY_LIST_NODE_FN_CREATE(MY_LIST_NODE_STRUCT* node) {
    if (!node) {
        MY_CALLOC(node, MY_LIST_NODE_STRUCT, 1);
        node->allocated = true;
    } else {
        node->allocated = false;
    }

    node->data = (MY_LIST_DATA_TYPE)(0);
    node->next = NULL;
    node->prev = NULL;
    node->list = NULL;

    return node;
}
MY_LIST_NODE_STRUCT* MY_LIST_NODE_FN_DUPLICATE(MY_LIST_NODE_STRUCT* src, MY_LIST_NODE_STRUCT* dst) {
    MY_ASSERT_PTR(src);

    dst = MY_LIST_NODE_FN_CREATE(dst);
    dst->data = src->data;

    return dst;
}
void MY_LIST_NODE_FN_DESTROY(MY_LIST_NODE_STRUCT* node) {
    MY_ASSERT_PTR(node);
    MY_ASSERT(node->list == NULL, "Destroying an attached node (HINT: Set deallocate to true in any list erasing function)");

    if (node->allocated) {
        MY_FREE(node);
    }
}

void MY_LIST_NODE_FN_SET(MY_LIST_NODE_STRUCT* node, MY_LIST_DATA_TYPE value) {
    MY_ASSERT_PTR(node);
    node->data = value;
}
MY_LIST_DATA_TYPE MY_LIST_NODE_FN_GET(MY_LIST_NODE_STRUCT* node) {
    MY_ASSERT_PTR(node);
    return node->data;
}
MY_LIST_NODE_STRUCT* MY_LIST_NODE_FN_NEXT(MY_LIST_NODE_STRUCT* node) {
    MY_ASSERT_PTR(node);
    return node->next;
}
MY_LIST_NODE_STRUCT* MY_LIST_NODE_FN_PREV(MY_LIST_NODE_STRUCT* node) {
    MY_ASSERT_PTR(node);
    return node->prev;
}
MY_LIST_STRUCT* MY_LIST_NODE_FN_LIST(MY_LIST_NODE_STRUCT* node) {
    MY_ASSERT_PTR(node);
    return node->list;
}

#endif /* MY_LIST_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#undef MY_LIST_NAME
#undef MY_LIST_FN_PREFIX
#undef MY_LIST_DATA_TYPE

#undef MY_LIST_STRUCT
#undef MY_LIST_NODE_STRUCT

#undef MY_LIST_FN_CREATE
#undef MY_LIST_FN_DESTROY

#undef MY_LIST_FN_RDLOCK
#undef MY_LIST_FN_WRLOCK
#undef MY_LIST_FN_RDUNLOCK
#undef MY_LIST_FN_WRUNLOCK

#undef MY_LIST_FN_GET
#undef MY_LIST_FN_SIZE
#undef MY_LIST_FN_BACK
#undef MY_LIST_FN_FRONT

#undef MY_LIST_FN_CLEAR
#undef MY_LIST_FN_ERASE
#undef MY_LIST_FN_POP_BACK
#undef MY_LIST_FN_POP_FRONT

#undef MY_LIST_FN_INSERT_NEXT
#undef MY_LIST_FN_INSERT_PREV
#undef MY_LIST_FN_PUSH_BACK
#undef MY_LIST_FN_PUSH_FRONT

#undef MY_LIST_NODE_FN_CREATE
#undef MY_LIST_NODE_FN_DUPLICATE
#undef MY_LIST_NODE_FN_DESTROY

#undef MY_LIST_NODE_FN_SET
#undef MY_LIST_NODE_FN_GET
#undef MY_LIST_NODE_FN_NEXT
#undef MY_LIST_NODE_FN_PREV
#undef MY_LIST_NODE_FN_LIST