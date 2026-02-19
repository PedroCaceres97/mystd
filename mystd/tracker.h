#ifndef __MYSTD_TRACKER_H__
#define __MYSTD_TRACKER_H__ 

#include <mystd/stdio.h>

#ifndef MY_TRACKER_FREE
    #define MY_TRACKER_FREE(ptr)                   do { free((void*)(ptr)); (ptr) = NULL;                                                                       } while(0)
#endif /* MY_TRACKER_FREE */
#ifndef MY_TRACKER_MALLOC
    #define MY_TRACKER_MALLOC(v, type, size)       do { (v) = (type*)malloc((size));                MY_ASSERT_MALLOC((v), type, (size)); memset(v, 0, (size));  } while(0)
#endif /* MY_TRACKER_MALLOC */
#ifndef MY_TRACKER_CALLOC
    #define MY_TRACKER_CALLOC(v, type, count)      do { (v) = (type*)calloc((count), sizeof(type)); MY_ASSERT_CALLOC((v), type, (count));                       } while(0)
#endif /* MY_TRACKER_CALLOC */

#ifdef __cplusplus
extern "C" {
#endif

struct MyTracker;
struct MyTrackerPtrhdr;
struct MyTrackerNode;
struct MyTrackerList;

typedef struct MyTracker MyTracker;
typedef struct MyTrackerPtrhdr MyTrackerPtrhdr;
typedef struct MyTrackerNode MyTrackerNode;
typedef struct MyTrackerList MyTrackerList;

MyTracker*          MyTracker_Create    (MyTracker* tracker, MyContext context);
void                MyTracker_Destroy   (MyTracker* tracker);

void                MyTracker_Rdlock    (MyTracker* tracker);
void                MyTracker_Wrlock    (MyTracker* tracker);
void                MyTracker_Rdunlock  (MyTracker* tracker);
void                MyTracker_Wrunlock  (MyTracker* tracker);

void                MyTracker_Clear     (MyTracker* tracker);
void                MyTracker_Free      (MyTracker* tracker, void* ptr);
void*               MyTracker_Alloc     (MyTracker* tracker, MyContext context, size_t size);
void*               MyTracker_Realloc   (MyTracker* tracker, MyContext context, void* ptr, size_t size);
void*               MyTracker_Duplicate (MyTracker* tracker, MyContext context, void* ptr, size_t size);

size_t              MyTracker_Bytes     (MyTracker* tracker);
size_t              MyTracker_Count     (MyTracker* tracker);
void                MyTracker_Dump      (MyTracker* tracker, MyFile* file);

struct MyTrackerNode {
    MyTrackerPtrhdr*       data;
    struct MyTrackerNode*  next;
    struct MyTrackerNode*  prev;
};
struct MyTrackerList {
    size_t                          size;
    struct MyTrackerNode*  back;
    struct MyTrackerNode*  front;
};

struct MyTrackerPtrhdr {
    MyTracker*        tracker;
    MyTrackerNode    node;
    size_t                    size;
    MyContext                 context;
};
struct MyTracker {
    MY_RWLOCK_TYPE            lock;
    MyTrackerList    ptrs;
    size_t                    bytes;
    MyContext                 context;
    int                       allocated;
};

#ifdef __cplusplus
}
#endif

#endif /* __MYSTD_MY_TRACKER_H__ */