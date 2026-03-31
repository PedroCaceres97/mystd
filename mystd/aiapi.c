#include <mystd/aiapi.h>

#define MY_AIAPI_OPENAI_DEFAULT_URL          "https://api.openai.com/v1/chat/completions"
#define MY_AIAPI_OPENAI_DEFAULT_HISTORY      15
#define MY_AIAPI_OPENAI_DEFAULT_REMOVE_IDX   0
#define MY_AIAPI_OPENAI_DEFAULT_MAXTOKENS    200
#define MY_AIAPI_OPENAI_DEFAULT_TEMPERATURE  0.5f
#define MY_AIAPI_OPENAI_DEFAULT_TOP_P        0.5f

#define MY_AIAPI_OPENROUTER_DEFAULT_URL      "https://openrouter.ai/api/v1/chat/completions"
#define MY_AIAPI_OPENROUTER_DEFAULT_X_TITLE  "mystd default x-title"

#ifdef __cplusplus
extern "C" {
#endif

static size_t MyAIAPI_WriteCallback(void* data, size_t size, size_t nmemb, void* userp) {
    MY_ASSERT_PTR(data);
    MY_ASSERT_PTR(userp);

    MyAIAPI* api = (MyAIAPI*)userp;
    size_t total = size * nmemb;
    MY_REALLOC(api->buffer, char, api->buffer, api->written + total + 1);
    memcpy(api->buffer + api->written, data, total);
    api->written += total;
    api->buffer[api->written] = '\0';
    return total;
}
static void MyAIAPI_AppendHdr(struct curl_slist** header, const char* format, ...) {
    MY_ASSERT_PTR(header);
    MY_ASSERT_PTR(format);

    char buffer[2048] = {0};
    va_list args;
    va_start(args, format);
    MyVsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    *header = curl_slist_append(*header, buffer);
    MY_ASSERT(*header != NULL, MySprintf("curl_slist_append failed -> %s", buffer));
}

static void MyAIAPIOpenai_InitHeaders(MyAIAPI* api) {
    MyAIAPI_AppendHdr(&api->headers, "Authorization: Bearer %s", api->config.apiKey);
    MyAIAPI_AppendHdr(&api->headers, "Content-Type: application/json");
}
static void MyAIAPIOpenrouter_InitHeaders(MyAIAPI* api) {
    MyAIAPI_AppendHdr(&api->headers, "Authorization: Bearer %s", api->config.apiKey);
    MyAIAPI_AppendHdr(&api->headers, "Content-Type: application/json");
    if (api->config.httpReferer) {
       MyAIAPI_AppendHdr(&api->headers, "HTTP-Referer: %s", api->config.httpReferer);
    }
    if (api->config.xtitle) {
       MyAIAPI_AppendHdr(&api->headers, "X-Title: %s", api->config.xtitle);
    }
}
static void MyAIAPILlamaCpp_InitHeaders(MyAIAPI* api) {
    MyAIAPI_AppendHdr(&api->headers, "Content-Type: application/json");
}

static bool backendSupportsSeed[] = {
    [MY_AIAPI_OPENAI]       = true,
    [MY_AIAPI_LLAMA_CPP]    = false,
    [MY_AIAPI_OPENROUTER]   = true
};
static void (*backendInitHeaders[])(MyAIAPI*) = {
    [MY_AIAPI_OPENAI]       = MyAIAPIOpenai_InitHeaders,
    [MY_AIAPI_LLAMA_CPP]    = MyAIAPILlamaCpp_InitHeaders,
    [MY_AIAPI_OPENROUTER]   = MyAIAPIOpenrouter_InitHeaders
};

static const char* roleToString[] = {
    [MY_AIAPI_ROLE_USER] = "user",
    [MY_AIAPI_ROLE_SYSTEM] = "system",
    [MY_AIAPI_ROLE_ASSISTANT] = "assistant"
};

static MyAIAPIRole StringToRole(const char* string) {
    if (strcmp(string, "user") == 0) { return MY_AIAPI_ROLE_USER; }
    if (strcmp(string, "system") == 0) { return MY_AIAPI_ROLE_SYSTEM; }
    if (strcmp(string, "assistant") == 0) { return MY_AIAPI_ROLE_ASSISTANT; }
    MyLog(MY_FATAL, "String is not a role -> %s", string);
    return MY_AIAPI_ROLE_USER;
}

MyAIAPIChatConfig MyAIAPI_DefaultChatConfig(int removeIndex) {
    MyAIAPIChatConfig config = {0};
    config.seed             = -1;
    config.reasoning        = -1;
    config.maxHistory       = 30;
    config.removeIndex      = removeIndex;
    config.maxTokens        = 200;
    config.temperature      = 0.5f;
    config.topp             = 0.5f;
    config.presencePenalty  = 0.0f;
    config.frequencyPenalty = 0.0f;
    return config;
}

MyAIAPI*        MyAIAPI_Create      (MyAIAPI* api, MyAIAPIConfig config) {
    MY_ASSERT(config.model != NULL, "AI model cant be NULL");
    MY_STRUCT_CREATE_RULE(api, MyAIAPI);
    
    api->config = config;
    if (api->config.backend == MY_AIAPI_OPENAI) {
        if (!api->config.url) { api->config.url = MY_AIAPI_OPENAI_DEFAULT_URL; }
        MY_ASSERT(api->config.apiKey, "When using MY_AIAPI_OPENAI apiKey cant be NULL");
    } else if (config.backend == MY_AIAPI_LLAMA_CPP) {
        MY_ASSERT(api->config.url, "When using MY_AIAPI_LLAMA_CPP url cant be NULL");
    } else if (config.backend == MY_AIAPI_OPENROUTER) {
        if (!api->config.url) { api->config.url = MY_AIAPI_OPENROUTER_DEFAULT_URL; }
        if (!api->config.xtitle) { api->config.xtitle = MY_AIAPI_OPENROUTER_DEFAULT_X_TITLE; }
        MY_ASSERT(api->config.apiKey, "When using MY_AIAPI_OPENROUTER apiKey cant be NULL");
    } else {
        MY_ASSERT(false, "Unknown MyAIAPI backend");
    }

    api->curl = curl_easy_init();
    MY_ASSERT(api->curl, "curl_easy_init failed");
    backendInitHeaders[api->config.backend](api);

    CURLcode cres = curl_easy_setopt(api->curl, CURLOPT_POST, 1L);
    MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_POST failed: %s", curl_easy_strerror(cres)));

    return api;
}
void            MyAIAPI_Destroy     (MyAIAPI* api) {
    MY_ASSERT_PTR(api);
    if (api->headers) { curl_slist_free_all(api->headers); }
    if (api->curl) { curl_easy_cleanup(api->curl); }
    MY_FREE_IF(api->buffer);
    MY_STRUCT_DESTROY_RULE(api);
}
const char*     MyAIAPI_Submit      (MyAIAPI* api, MyAIAPIChat* chat) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_PTR(api->curl);
    MY_ASSERT_PTR(chat);
    MY_ASSERT_PTR(chat->messages);

    MyJson_KSetString(chat->root.body, "model", api->config.model);
    if (chat->config.seed >= 0 && backendSupportsSeed[api->config.backend]) {
        MyJson_KSetInteger(chat->root.body, "seed", chat->config.seed);
    }

    api->written = 0;
    MY_FREE_IF(api->buffer);

    char* payload = MyJsonRoot_Print(&chat->root, false);

    CURLcode cres;
    cres = curl_easy_setopt(api->curl, CURLOPT_URL, api->config.url);
    MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_URL failed: %s", curl_easy_strerror(cres)));
    cres = curl_easy_setopt(api->curl, CURLOPT_HTTPHEADER, api->headers);
    MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_HTTPHEADER failed: %s", curl_easy_strerror(cres)));
    cres = curl_easy_setopt(api->curl, CURLOPT_POSTFIELDS, payload);
    MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_POSTFIELDS failed: %s", curl_easy_strerror(cres)));
    cres = curl_easy_setopt(api->curl, CURLOPT_POSTFIELDSIZE, (long)strlen(payload));
    MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_POSTFIELDSIZE failed: %s", curl_easy_strerror(cres)));
    cres = curl_easy_setopt(api->curl, CURLOPT_WRITEFUNCTION, MyAIAPI_WriteCallback);
    MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_WRITEFUNCTION failed: %s", curl_easy_strerror(cres)));
    cres = curl_easy_setopt(api->curl, CURLOPT_WRITEDATA, api);
    MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_WRITEDATA failed: %s", curl_easy_strerror(cres)));

    if (api->config.timeoutms > 0) {
        cres = curl_easy_setopt(api->curl, CURLOPT_TIMEOUT_MS, api->config.timeoutms);
        MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_TIMEOUT_MS failed: %s", curl_easy_strerror(cres)));
    }
    if (api->config.cainfoPath) {
        cres = curl_easy_setopt(api->curl, CURLOPT_CAINFO, api->config.cainfoPath);
        MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_CAINFO failed: %s", curl_easy_strerror(cres)));
    }

    cres = curl_easy_perform(api->curl);
    MY_FREE(payload);
    
    MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_perform failed: %s", curl_easy_strerror(cres)));
    MY_ASSERT(api->written > 0, "Empty HTTP response");
    long status = 0;
    curl_easy_getinfo(api->curl, CURLINFO_RESPONSE_CODE, &status);
    MY_ASSERT(status >= 200 && status < 300, "HTTP error: %ld\n%s", status, api->buffer);

    MyJsonRoot response = {0};
    MyJsonRoot_Parse(&response, api->buffer);
    
    MyJson* error = MyJson_KGet(response.body, "error");
    if (error != NULL) {
        MyLog(MY_ERROR, "Printing response root (error founded)");
        char* parsed = MyJsonRoot_Print(&response, true);
        MyFilePrint(MY_LOG_STDERR_FILE, parsed);
        MyFilePrint(MY_LOG_STDERR_FILE, "\n\n");
        MyLog(MY_FATAL, "Yeah, nothing to do bro");
    }

    char* answer = NULL;
    MyJson* choice = MyJson_KGetPath(response.body, "choices[0]");
    MyJson* message = MyJson_KGet(choice, "message");
    if (!message) {
        /* llama.cpp fallback */
        MyJson* text = MyJson_KGet(choice, "text");
        MY_ASSERT(text && MyJson_IsString(text), "Invalid llama.cpp response");
        answer = MyJson_String(text);
    } else {
        answer = MyJson_KGetString(message, "content");
    }

    MyJson* copy = MyJson_PushObject(chat->messages);
    MyJson_KSetString(copy, "role", "assistant");
    MyJson_KSetString(copy, "content", answer);
    MyJsonRoot_Destroy(&response);
    if (MyJson_ArraySize(chat->messages) > chat->config.maxHistory) {
        MyJson_ArrayRemove(chat->messages, chat->config.removeIndex);
    }
    return (const char*)MyJson_KGetString(copy, "content");
}

MyAIAPIChat*    MyAIAPIChat_Create  (MyAIAPIChat* chat, MyAIAPIChatConfig config) {
    MY_STRUCT_CREATE_RULE(chat, MyAIAPIChat);

    chat->config = config;
    chat->config.maxHistory     = MY_TERNARY(config.maxHistory  > 0,                             config.maxHistory,     MY_AIAPI_OPENAI_DEFAULT_HISTORY);
    chat->config.removeIndex    = MY_TERNARY(config.removeIndex >= 0,                            config.removeIndex,    MY_AIAPI_OPENAI_DEFAULT_REMOVE_IDX);
    chat->config.maxTokens      = MY_TERNARY((config.maxTokens  > 0 || config.maxTokens == -1),  config.maxTokens,      MY_AIAPI_OPENAI_DEFAULT_MAXTOKENS);
    chat->config.temperature    = MY_TERNARY(config.temperature >= 0.0f,                         config.temperature,    MY_AIAPI_OPENAI_DEFAULT_TEMPERATURE);
    chat->config.topp           = MY_TERNARY(config.topp        >= 0.0f,                         config.topp,           MY_AIAPI_OPENAI_DEFAULT_TOP_P);

    MyJsonRoot_Create(&chat->root, MY_JSON_OBJECT);
    chat->messages = MyJson_KSetArray(chat->root.body, "messages");
    MyJson_KSetDecimal(chat->root.body, "top_p", chat->config.topp);
    MyJson_KSetDecimal(chat->root.body, "temperature", chat->config.temperature);
    if (chat->config.maxTokens != -1) {
        MyJson_KSetInteger(chat->root.body, "max_tokens", chat->config.maxTokens);
    }
    if (chat->config.presencePenalty != 0.0f) { 
        MyJson_KSetDecimal(chat->root.body, "presence_penalty", chat->config.presencePenalty);
    }
    if (chat->config.frequencyPenalty != 0.0f) { 
        MyJson_KSetDecimal(chat->root.body, "frequency_penalty", chat->config.frequencyPenalty);
    }

    return chat;
}
void            MyAIAPIChat_Destroy (MyAIAPIChat* chat) {
    MY_ASSERT_PTR(chat);
    MyJsonRoot_Destroy(&chat->root);
    MY_STRUCT_DESTROY_RULE(chat);
}

size_t          MyAIAPIChat_Size    (MyAIAPIChat* chat) {
    MY_ASSERT_PTR(chat);
    return MyJson_ArraySize(chat->messages);
}
MyAIAPIMsg      MyAIAPIChat_Get     (MyAIAPIChat* chat, size_t index) {
    MY_ASSERT_PTR(chat);
    MyJson* message = MyJson_GetObject(chat->messages, index);
    return (MyAIAPIMsg){.text = MyJson_KGetString(message, "content"), .role = StringToRole(MyJson_KGetString(message, "role"))};
}
void            MyAIAPIChat_Set     (MyAIAPIChat* chat, MyAIAPIMsg msg, size_t index) {
    MY_ASSERT_PTR(chat);
    MyJson* message = MyJson_SetObject(chat->messages, index);
    MyJson_KSetString(message, "role", roleToString[msg.role]);
    MyJson_KSetString(message, "content", msg.text);
}
void            MyAIAPIChat_Push    (MyAIAPIChat* chat, MyAIAPIMsg msg) {
    MY_ASSERT_PTR(chat);
    MyJson* message = MyJson_PushObject(chat->messages);
    MyJson_KSetString(message, "role", roleToString[msg.role]);
    MyJson_KSetString(message, "content", msg.text);
    if (MyJson_ArraySize(chat->messages) > chat->config.maxHistory) {
        MyJson_ArrayRemove(chat->messages, chat->config.removeIndex);
    }
}
void            MyAIAPIChat_Insert  (MyAIAPIChat* chat, MyAIAPIMsg msg, size_t index) {
    MY_ASSERT_PTR(chat);
    MyJson* message = MyJson_InsertObject(chat->messages, index);
    MyJson_KSetString(message, "role", roleToString[msg.role]);
    MyJson_KSetString(message, "content", msg.text);
    if (MyJson_ArraySize(chat->messages) > chat->config.maxHistory) {
        MyJson_ArrayRemove(chat->messages, chat->config.removeIndex);
    }
}
void            MyAIAPIChat_Erase   (MyAIAPIChat* chat, size_t index) {
    MY_ASSERT_PTR(chat);
    MyJson_ArrayRemove(chat->messages, index);
}

#ifdef __cplusplus
}
#endif