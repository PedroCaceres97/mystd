#ifndef __MYSTD_AIAPI_H__
#define __MYSTD_AIAPI_H__

#include <mystd/stdlib.h>
#include <mystd/json.h>

#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MY_AIAPI_OPENAI,
    MY_AIAPI_LLAMA_CPP,
    MY_AIAPI_OPENROUTER
} MyAIAPIBackend;

typedef enum {
    MY_AIAPI_ROLE_USER,
    MY_AIAPI_ROLE_SYSTEM,
    MY_AIAPI_ROLE_ASSISTANT
} MyAIAPIRole;

typedef struct {
    const char* text;
    MyAIAPIRole role;
} MyAIAPIMsg;

/*
    const MyAIAPIConfig apiConfig = {
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
        .removeIndex       = 0,
        .maxTokens          = 0,
        .temperature        = 0.5f,
        .topp               = 0.5f,
        .presencePenalty    = 0.0f,
        .frequencyPenalty   = 0.0f,
    };
*/
typedef struct {
    int     seed;               /* -1       = unset */
    int     reasoning;          /* -1       = no data send */
    int     maxTokens;          /* 0        = default (200 tokens) | -1 = no data send*/
    float   temperature;        /* -1.0f    = default (0.5f) */
    float   topp;               /* -1.0f    = default (0.5f) */
    float   presencePenalty;    /*  0       = no effect */
    float   frequencyPenalty;   /*  0       = no effect */
    size_t  maxHistory;         /* -1       = default (30) */
    size_t  removeIndex;        /* -1       = default (0) | Index used to remove chat messages when history sizes reaches maxHistory, this allows keeping system messages */
} MyAIAPIChatConfig;

MyAIAPIChatConfig MyAIAPI_DefaultChatConfig(int removeIndex);

typedef struct {
    MyStructHeader              header;

    MyAIAPIChatConfig           config;
    MyJsonRoot                  root;
    MyJson*                     messages;
} MyAIAPIChat;

MY_RWLOCK_DECLARES(MyAIAPI, api, MyAIAPI)

#define AI_USER(msg) (MyAIAPIMsg){ .role = MY_AIAPI_ROLE_USER, .text = msg }
#define AI_SYSTEM(msg) (MyAIAPIMsg){ .role = MY_AIAPI_ROLE_SYSTEM, .text = msg }
#define AI_ASSISTANT(msg) (MyAIAPIMsg){ .role = MY_AIAPI_ROLE_ASSISTANT, .text = msg }

MyAIAPI*        MyAIAPI_Create      (MyAIAPI* api, MyAIAPIConfig config);
void            MyAIAPI_Destroy     (MyAIAPI* api);
const char*     MyAIAPI_Submit     (MyAIAPI* api, MyAIAPIChat* chat);

MyAIAPIMsg      MyAIAPI_User        (const char* msg);
MyAIAPIMsg      MyAIAPI_System      (const char* msg);
MyAIAPIMsg      MyAIAPI_Assistant   (const char* msg);

MyAIAPIChat*    MyAIAPIChat_Create  (MyAIAPIChat* chat, MyAIAPIChatConfig config);
void            MyAIAPIChat_Destroy (MyAIAPIChat* chat);

size_t          MyAIAPIChat_Size    (MyAIAPIChat* chat);
MyAIAPIMsg      MyAIAPIChat_Get     (MyAIAPIChat* chat, size_t index);
void            MyAIAPIChat_Set     (MyAIAPIChat* chat, MyAIAPIMsg msg, size_t index);
void            MyAIAPIChat_Push    (MyAIAPIChat* chat, MyAIAPIMsg msg);
void            MyAIAPIChat_Insert  (MyAIAPIChat* chat, MyAIAPIMsg msg, size_t index);
void            MyAIAPIChat_Erase   (MyAIAPIChat* chat, size_t index);

#ifdef __cplusplus
}
#endif

#endif /* __MYSTD_AIAPI_H__ */