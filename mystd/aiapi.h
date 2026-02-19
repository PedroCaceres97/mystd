#ifndef __MYSTD_AIAPI_H__
#define __MYSTD_AIAPI_H__

#include <mystd/stdio.h>
#include <mystd/stdlib.h>

#include <curl/curl.h>
#include <cJSON/cJSON.h>

#ifndef MY_AIAPI_TOOL_COUNT
  #define MY_AIAPI_TOOL_COUNT 32
#endif /* MY_AIAPI_TOOL_COUNT */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MyAIAPIBackend {
  MY_AIAPI_OPENAI,
  MY_AIAPI_OPENROUTER,
  MY_AIAPI_LLAMA_CPP
} MyAIAPIBackend;

struct MyAIAPI;
struct MyAIAPITool;
struct MyAIAPIConfig;
struct MyAIAPIBackendOps;

typedef struct MyAIAPI MyAIAPI;
typedef struct MyAIAPITool MyAIAPITool;
typedef struct MyAIAPIConfig MyAIAPIConfig;
typedef struct MyAIAPIBackendOps MyAIAPIBackendOps;

/* 
 * Tool function must return a newly allocated cJSON*.
 * Ownership is transferred to Openai and it will be deleted.
 * Do not cJSON_Delete(args)
 */
typedef cJSON* (*MyAIAPIToolFn)(cJSON* args);

MyAIAPI*                MyAIAPI_Create          (MyAIAPI* api, MyAIAPIConfig config);
void                    MyAIAPI_Destroy         (MyAIAPI* api);
cJSON*                  MyAIAPI_Send            (MyAIAPI* api, cJSON* message);

void                    MyAIAPI_Rdlock          (MyAIAPI* api);
void                    MyAIAPI_Wrlock          (MyAIAPI* api);
void                    MyAIAPI_Rdunlock        (MyAIAPI* api);
void                    MyAIAPI_Wrunlock        (MyAIAPI* api);

void                    MyAIAPI_ToolCreate     (MyAIAPITool* tool, const char* name, const char* description, MyAIAPIToolFn fn);
void                    MyAIAPI_ToolAddParam  (MyAIAPITool* tool, const char* name, const char* type);
void                    MyAIAPI_ToolAttach     (MyAIAPI* api, size_t idx, MyAIAPITool tool);
void                    MyAIAPI_ToolDetach     (MyAIAPI* api, size_t idx);

void                    MyAIAPI_HistorySet     (MyAIAPI* api, size_t idx, cJSON* message);
cJSON*                  MyAIAPI_HistoryGet     (MyAIAPI* api, size_t idx);
void                    MyAIAPI_HistoryErase   (MyAIAPI* api, size_t idx);
void                    MyAIAPI_HistoryInsert  (MyAIAPI* api, size_t idx, cJSON* message);
int                     MyAIAPI_HistorySize    (MyAIAPI* api);

struct MyAIAPIBackendOps {
    void    (*init_request)     (MyAIAPI*);
    void    (*init_headers)     (MyAIAPI*);
    bool    (*parse_tool_args)  (cJSON* raw, cJSON** out);
    void    (*push_tool_result) (MyAIAPI*, const char*, const char*, cJSON*);
    bool    (*supports_seed)    (void);
};

struct MyAIAPIConfig {
    MyAIAPIBackend backend;

    const char* url;                /* API endpoint; default provided below */
    const char* model;              /* model id */
    const char* api_key;            /* required */
    const char* x_title;            /* optional X-Title header */
    const char* http_referer;       /* optional Referer header */
    const char* cainfo_path;        /* optional path to cacert.pem */

    int   seed;                     /* -1 = unset */
    int   reasoning;                /* -1 = no data send */
    int   max_history;              /* -1     = default (15 messages) */
    int   max_remove_idx;           /* idx to remove message when history gets to its max, this allows to keep system message without weird logic */
    int   max_tokens;               /* 0      = default (200 tokens) | -1 = no data send*/
    long  timeout_ms;               /*  0     = default (no explicit timeout) */
    float temperature;              /* -1.0f  = default (0.5f) */
    float top_p;                    /* -1.0f  = default (0.5f) */
    float presence_penalty;         /*  0     = no effect */
    float frequency_penalty;        /*  0     = no effect */
    cJSON* stop;                    /* array or string (owned by caller) */
    cJSON* response_format;         /* JSON schema / type */
    cJSON* logit_bias;              /* object */    
};

struct MyAIAPITool {
    bool            in_use;
    const char*     name;
    const char*     description;
    cJSON*          parameters;
    MyAIAPIToolFn   fn;
};

struct MyAIAPI {
    int allocated;
    MY_RWLOCK_TYPE lock;

    cJSON* root;
    cJSON* tools;
    cJSON* messages;

    CURL* curl;
    struct curl_slist* headers;

    char* response;
    size_t response_size;

    MyAIAPITool attached[MY_AIAPI_TOOL_COUNT];
    MyAIAPIConfig config;
};

#ifdef __cplusplus
}
#endif

#endif /* __MYSTD_AIAPI_H__ */