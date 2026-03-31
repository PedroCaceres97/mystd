#ifndef __MYSTD_AIAPI_H__
#define __MYSTD_AIAPI_H__

#include <mystd/stdlib.h>

#include <curl/curl.h>
#include <cJSON/cJSON.h>

/*

MyAIAPI MyAIAPI_Create();
void    MyAIAPI_Destroy();
void    MyAIAPI_Submit(MyAIAPIChat* chat);

MyAIAPIChat MyAIAPIChat_Create();
void        MyAIAPIChat_Destroy();

void        MyAIAPIChat_InsertUser();
void        MyAIAPIChat_InsertSystem();

*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MY_AIAPI_OPENAI,
    MY_AIAPI_OPENROUTER,
    MY_AIAPI_LLAMA_CPP
} MyAIAPIBackend;

typedef enum {
    MY_AIAPI_ROLE_SYSTEM,
    MY_AIAPI_ROLE_ASSISTANT,
    MY_AIAPI_ROLE_USER
} MyAIAPIChatRole;

typedef struct {
    const char* text;
    MyAIAPIChatRole role;
} MyAIAPIChatMsg;

#define MY_VECTOR_NAME              MyVecAIAPIChatMsg
#define MY_VECTOR_DATA_TYPE         MyAIAPIChatMsg
#ifdef MY_AIAPI_VECTOR
    #define MY_VECTOR_INITIAL_SIZE  30
    #define MY_VECTOR_IMPLEMENTATION
#endif
#include <mystd/vector.h>

/*
    const MyAIAPIChatConfig aiapiConfig = {
        .backend        = [BACKEND],
        .url            = [URL],
        .model          = [BACKEND],
        .apiKey         = [API KEY],
        .xtitle         = NULL,
        .httpReferer    = NULL,
        .cainfoPath     = NULL,
        .timeoutms      = 0,
    };
*/
typedef struct {
    MyAIAPIBackend  backend;
    const char*     url;                /* API endpoint; default provided below */
    const char*     model;              /* model id */
    const char*     apiKey;             /* required */
    const char*     xtitle;             /* optional X-Title header */
    const char*     httpReferer;        /* optional Referer header */
    const char*     cainfoPath;         /* optional path to cacert.pem */
    long            timeoutms;          /*  0     = default (no explicit timeout) */
} MyAIAPIConfig;

typedef struct {
    MyStructHeader              header;

    struct curl_slist*          headers;
    CURL*                       curl;
    char*                       buffer;
    size_t                      written;

    MyAIAPIConfig               config;
} MyAIAPI;

/*
    const MyAIAPIChatConfig chatConfig = {
        .seed               = -1,
        .reasoning          = -1,
        .maxHistory         = -1,
        .maxRemoveIdx       = 0,
        .maxTokens          = 0,
        .temperature        = 0.5f,
        .topp               = 0.5f,
        .presencePenalty    = 0,
        .frequencyPenalty   = NULL,
        .stop               = NULL,
        .responseFormat     = NULL,
        .logitBias          = NULL
    };
*/
typedef struct {
    int             seed;               /* -1 = unset */
    int             reasoning;          /* -1 = no data send */
    int             maxHistory;         /* -1     = default (15 messages) */
    int             maxRemoveIdx;       /* idx to remove message when history gets to its max, this allows to keep system message without weird logic */
    int             maxTokens;          /* 0      = default (200 tokens) | -1 = no data send*/
    float           temperature;        /* -1.0f  = default (0.5f) */
    float           topp;               /* -1.0f  = default (0.5f) */
    float           presencePenalty;    /*  0     = no effect */
    float           frequencyPenalty;   /*  0     = no effect */
    cJSON*          stop;               /* array or string (owned by caller) */
    cJSON*          responseFormat;     /* JSON schema / type */
    cJSON*          logitBias;          /* object */    
} MyAIAPIChatConfig;

typedef struct {
    MyStructHeader              header;

    cJSON*                      root;
    cJSON*                      tools;
    cJSON*                      messages;
    MyAIAPIChatConfig*          config;
    MyVecAIAPIChatMsg           history;
} MyAIAPIChat;

MY_RWLOCK_DECLARES(MyAIAPI, api, MyAIAPI)

MyAIAPI*        MyAIAPI_Create      (MyAIAPI* api, MyAIAPIConfig config);
void            MyAIAPI_Destroy     (MyAIAPI* api);
void            MyAIAPI_Submmit     (MyAIAPI* api, MyAIAPIChat* chat);

MyAIAPIChat*    MyAIAPIChat_Create  (MyAIAPIChat* chat, MyAIAPIChatConfig config);
void            MyAIAPIChat_Destroy (MyAIAPIChat* chat);

MyAIAPIChatMsg  MyAIAPIChat_Get     (MyAIAPIChat* chat, size_t idx);
void            MyAIAPIChat_Set     (MyAIAPIChat* chat, MyAIAPIChatMsg msg, size_t idx);
void            MyAIAPIChat_Push    (MyAIAPIChat* chat, MyAIAPIChatMsg msg);
void            MyAIAPIChat_Insert  (MyAIAPIChat* chat, MyAIAPIChatMsg msg, size_t idx);
void            MyAIAPIChat_Erase   (MyAIAPIChat* chat, size_t idx);

#ifdef __cplusplus
}
#endif

#endif /* __MYSTD_AIAPI_H__ */