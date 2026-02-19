#include <mystd/tracker.h>

#define MY_TRACKER_PTR_TO_HDR(ptr) ((MyTrackerPtrhdr*)  MY_PTR_SUB(ptr, sizeof(struct MyTrackerPtrhdr)))
#define MY_TRACKER_HDR_TO_PTR(hdr) ((void*)                      MY_PTR_ADD(hdr, sizeof(struct MyTrackerPtrhdr)))

static MyTrackerNode* MyTrackerList_Insert(MyTrackerList* list, MyTrackerNode* pivot, MyTrackerNode* node) {
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
    return node;
}
static void MyTrackerList_Erase(MyTrackerList* list, MyTrackerNode* node) {
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

MyTracker*  MyTracker_Create(MyTracker* tracker, MyContext context) {
    if (!tracker) {
        MY_CALLOC(tracker, struct MyTracker, 1);
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
void        MyTracker_Destroy(MyTracker* tracker) {
    MY_ASSERT_PTR(tracker);
    MY_ASSERT(tracker->ptrs.size == 0, "Destroying non empty tracker (HINT: Call free all)");

    if (tracker->allocated) {
        MY_TRACKER_FREE(tracker);
    }
}

void        MyTracker_Rdlock(MyTracker* tracker) {
  MY_ASSERT_PTR(tracker);
  MY_RWLOCK_RDLOCK(tracker->lock);
}
void        MyTracker_Wrlock(MyTracker* tracker) {
  MY_ASSERT_PTR(tracker);
  MY_RWLOCK_WRLOCK(tracker->lock);
}
void        MyTracker_Rdunlock(MyTracker* tracker) {
  MY_ASSERT_PTR(tracker);
  MY_RWLOCK_RDUNLOCK(tracker->lock);
}
void        MyTracker_Wrunlock(MyTracker* tracker) {
  MY_ASSERT_PTR(tracker);
  MY_RWLOCK_WRUNLOCK(tracker->lock);
}

void        MyTracker_Clear(MyTracker* tracker) {
  MY_ASSERT_PTR(tracker);

  MyTrackerNode* current = tracker->ptrs.front;
  while (current != NULL) {
    MyTrackerPtrhdr* hdr = current->data;
    current = current->next;
    MyTrackerList_Erase(&tracker->ptrs, tracker->ptrs.front);
    tracker->bytes -= hdr->size;
    MY_TRACKER_FREE(hdr);
  }
}
void        MyTracker_Free(MyTracker* tracker, void* ptr) {
  MY_ASSERT_PTR(tracker);
  MY_ASSERT_PTR(ptr);

  MyTrackerPtrhdr* hdr = MY_TRACKER_PTR_TO_HDR(ptr);
  MY_ASSERT(hdr->tracker == tracker, "Freeing a foreign ptr");

  MyTrackerList_Erase(&tracker->ptrs, &hdr->node);
  tracker->bytes -= hdr->size;

  MY_TRACKER_FREE(hdr);
}
void*       MyTracker_Alloc(MyTracker* tracker, MyContext context, size_t size) {
  MY_ASSERT_PTR(tracker);
  MY_ASSERT(size != 0, "Requested allocation size is 0");

  MyTrackerPtrhdr* hdr = NULL;
  MY_TRACKER_MALLOC(hdr, struct MyTrackerPtrhdr, size + sizeof(struct MyTrackerPtrhdr));

  MyTrackerList_Insert(&tracker->ptrs, tracker->ptrs.back, &hdr->node);
  hdr->node.data = hdr;
  hdr->tracker = tracker;
  hdr->size = size;
  hdr->context = context;

  tracker->bytes += size;
  void* ptr = MY_TRACKER_HDR_TO_PTR(hdr);
  return ptr;
}
void*       MyTracker_Realloc(MyTracker* tracker, MyContext context, void* ptr, size_t size) {
  MY_ASSERT_PTR(tracker);
  MY_ASSERT_PTR(ptr);
  MY_ASSERT(size != 0, "Requested allocation size is 0");

  MyTrackerPtrhdr* srchdr = MY_TRACKER_PTR_TO_HDR(ptr);
  MY_ASSERT(srchdr->tracker == tracker, "Reallocating a foreign ptr");

  MyTrackerPtrhdr* newhdr = NULL;
  MY_TRACKER_MALLOC(newhdr, struct MyTrackerPtrhdr, size + sizeof(struct MyTrackerPtrhdr));

  MyTrackerList_Insert(&tracker->ptrs, &srchdr->node, &newhdr->node);
  MyTrackerList_Erase(&tracker->ptrs, &srchdr->node);
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
void*       MyTracker_Duplicate(MyTracker* tracker, MyContext context, void* ptr, size_t size) {
  MY_ASSERT_PTR(tracker);
  MY_ASSERT_PTR(ptr);
  MY_ASSERT(size != 0, "Requested allocation size is 0");

  MyTrackerPtrhdr* srchdr = MY_TRACKER_PTR_TO_HDR(ptr);
  MY_ASSERT(srchdr->tracker == tracker, "Duplicating a foreign ptr");

  MyTrackerPtrhdr* newhdr = NULL;
  MY_TRACKER_MALLOC(newhdr, struct MyTrackerPtrhdr, size + sizeof(struct MyTrackerPtrhdr));

  MyTrackerList_Insert(&tracker->ptrs, &srchdr->node, &newhdr->node);
  newhdr->node.data = newhdr;
  newhdr->tracker = tracker;
  newhdr->size = size;
  newhdr->context = context;

  tracker->bytes += size;
  void*  newptr = MY_TRACKER_HDR_TO_PTR(newhdr);
  memcpy(newptr, ptr, MY_MIN(size, srchdr->size));
  return newptr;
}

size_t      MyTracker_Bytes(MyTracker* tracker) {
  MY_ASSERT_PTR(tracker);
  return tracker->bytes;
}
size_t      MyTracker_Count(MyTracker* tracker) {
  MY_ASSERT_PTR(tracker);
  return tracker->ptrs.size;
}
void        MyTracker_Dump(MyTracker* tracker, MyFile* file) {
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

    MyTrackerNode* current = tracker->ptrs.front;
    MyFprintf(file, "%s", "Individual Pointer Information\n");
    size_t index = 0;
    while (current != NULL) {
        MyTrackerPtrhdr* hdr = current->data;
        
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