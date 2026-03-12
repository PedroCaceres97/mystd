#include "stddef.h"
#include <mystd/stdlib.h>

#ifndef MY_OPTIONAL_NAME
    #define MY_OPTIONAL_NAME MyOptionalInt
#endif /* MY_OPTIONAL_NAME */

#ifndef MY_OPTIONAL_FN_PREFIX
    #define MY_OPTIONAL_FN_PREFIX MY_OPTIONAL_NAME
#endif /* MY_OPTIONAL_FN_PREFIX */

#ifndef MY_OPTIONAL_DATA_TYPE
    #define MY_OPTIONAL_DATA_TYPE int
#endif /* MY_OPTIONAL_DATA_TYPE */

/** @cond doxygen_ignore */
#define MY_OPTIONAL_STRUCT          MY_OPTIONAL_NAME

#define MY_OPTIONAL_FN_CREATE       MY_CONCAT2(MY_OPTIONAL_FN_PREFIX, _Create)
#define MY_OPTIONAL_FN_DESTROY      MY_CONCAT2(MY_OPTIONAL_FN_PREFIX, _Destroy)

#define MY_OPTIONAL_FN_SET          MY_CONCAT2(MY_OPTIONAL_FN_PREFIX, _Set)
#define MY_OPTIONAL_FN_GET          MY_CONCAT2(MY_OPTIONAL_FN_PREFIX, _Get)
#define MY_OPTIONAL_FN_RESET        MY_CONCAT2(MY_OPTIONAL_FN_PREFIX, _Reset)
#define MY_OPTIONAL_FN_HAS_VALUE    MY_CONCAT2(MY_OPTIONAL_FN_PREFIX, _HasValue)
/** @endcond */

#ifdef __cplusplus
extern "C" {
#endif

struct MY_OPTIONAL_STRUCT;
typedef struct MY_OPTIONAL_STRUCT MY_OPTIONAL_STRUCT;

MY_RWLOCK_DECLARES(MY_OPTIONAL_STRUCT, optional, MY_OPTIONAL_FN_PREFIX)

MY_OPTIONAL_STRUCT*     MY_OPTIONAL_FN_CREATE       (MY_OPTIONAL_STRUCT* optional);
void                    MY_OPTIONAL_FN_DESTROY      (MY_OPTIONAL_STRUCT* optional);

void                    MY_OPTIONAL_FN_SET          (MY_OPTIONAL_STRUCT* optional, MY_OPTIONAL_DATA_TYPE value);
MY_OPTIONAL_DATA_TYPE   MY_OPTIONAL_FN_GET          (MY_OPTIONAL_STRUCT* optional);
void                    MY_OPTIONAL_FN_RESET        (MY_OPTIONAL_STRUCT* optional);
bool                    MY_OPTIONAL_FN_HAS_VALUE    (MY_OPTIONAL_STRUCT* optional);

struct MY_OPTIONAL_STRUCT {
    MyStructHeader          header;

    MY_OPTIONAL_DATA_TYPE   data;
    bool8                   has;
};

#ifdef MY_OPTIONAL_IMPLEMENTATION

MY_RWLOCK_DEFINES(MY_OPTIONAL_STRUCT, optional, MY_OPTIONAL_FN_PREFIX)

MY_OPTIONAL_STRUCT*     MY_OPTIONAL_FN_CREATE       (MY_OPTIONAL_STRUCT* optional) {
    MY_STRUCT_CREATE_RULE(optional, MY_OPTIONAL_STRUCT);
    optional->has = false;
    memset(&optional->data, 0, sizeof(optional->data));
    return optional;
}
void                    MY_OPTIONAL_FN_DESTROY      (MY_OPTIONAL_STRUCT* optional) {
    MY_ASSERT_PTR(optional);
    MY_STRUCT_DESTROY_RULE(optional);
}

void                    MY_OPTIONAL_FN_SET          (MY_OPTIONAL_STRUCT* optional, MY_OPTIONAL_DATA_TYPE value) {
    MY_ASSERT_PTR(optional);
    optional->has = true;
    optional->data = value;
}
MY_OPTIONAL_DATA_TYPE   MY_OPTIONAL_FN_GET          (MY_OPTIONAL_STRUCT* optional) {
    MY_ASSERT_PTR(optional);
    MY_ASSERT(optional->has, MY_STR(MY_OPTIONAL_STRUCT) " data was not setted");
    return optional->data;
}
void                    MY_OPTIONAL_FN_RESET        (MY_OPTIONAL_STRUCT* optional) {
    MY_ASSERT_PTR(optional);
    optional->has = false;
    memset(&optional->data, 0, sizeof(MY_OPTIONAL_DATA_TYPE));
}
bool                    MY_OPTIONAL_FN_HAS_VALUE    (MY_OPTIONAL_STRUCT* optional) {
    MY_ASSERT_PTR(optional);
    return optional->has;
}

#endif /* MY_OPTIONAL_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#undef MY_OPTIONAL_NAME
#undef MY_OPTIONAL_FN_PREFIX

#undef MY_OPTIONAL_DATA_TYPE
#undef MY_OPTIONAL_IMPLEMENTATION
 
#undef MY_OPTIONAL_STRUCT

#undef MY_OPTIONAL_FN_CREATE    
#undef MY_OPTIONAL_FN_DESTROY

#undef MY_OPTIONAL_FN_SET
#undef MY_OPTIONAL_FN_GET
#undef MY_OPTIONAL_FN_RESET
#undef MY_OPTIONAL_FN_HAS_VALUE
