#include <mystd/aiapi.h>

/* --------------------------------------------------------------------------
 * Default macros
 * -------------------------------------------------------------------------- */

#define MY_AIAPI_OPENAI_DEFAULT_URL          "https://api.openai.com/v1/chat/completions"
#define MY_AIAPI_OPENAI_DEFAULT_HISTORY      15
#define MY_AIAPI_OPENAI_DEFAULT_REMOVE_IDX   1
#define MY_AIAPI_OPENAI_DEFAULT_MAXTOKENS    200
#define MY_AIAPI_OPENAI_DEFAULT_TEMPERATURE  0.5f
#define MY_AIAPI_OPENAI_DEFAULT_TOP_P        0.5f

#define MY_AIAPI_OPENROUTER_DEFAULT_URL      "https://openrouter.ai/api/v1/chat/completions"
#define MY_AIAPI_OPENROUTER_DEFAULT_X_TITLE  "mystd default x-title"

/* --------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------- */

static size_t       MyAIAPI_WriteCallback(void* data, size_t size, size_t nmemb, void* userp) {
    MY_ASSERT_PTR(data);
    MY_ASSERT_PTR(userp);

    MyAIAPI* api = (MyAIAPI*)userp;

    size_t total = size * nmemb;
    
    MY_REALLOC(api->response, char, api->response, api->response_size + total + 1);
    memcpy(api->response + api->response_size, data, total);
    api->response_size += total;
    api->response[api->response_size] = '\0';

    return total;
}
static void         MyAIAPI_AppendHdr(struct curl_slist** header, const char* format, ...) {
    MY_ASSERT_PTR(header);
    MY_ASSERT_PTR(format);

    char openai_temp_buffer[1024] = {0};
    va_list args;
    va_start(args, format);
    vsnprintf(openai_temp_buffer, sizeof(openai_temp_buffer), format, args);
    va_end(args);
    
    *header = curl_slist_append(*header, openai_temp_buffer);
    MY_ASSERT(*header != NULL, MySprintf("curl_slist_append failed -> %s", openai_temp_buffer));
}
static void         MyAIAPI_PushTool(MyAIAPI* api, const char* name, const char* description, const cJSON* parameters) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_PTR(name);
    MY_ASSERT_PTR(description);
    MY_ASSERT_PTR(parameters);

    cJSON* tool = cJSON_CreateObject();
    MY_ASSERT_PTR(tool);

    cJSON* fn = cJSON_AddObjectToObject(tool, "function");
    MY_ASSERT_PTR(fn);

    MY_ASSERT_PTR(cJSON_AddStringToObject(tool, "type", "function"));
    MY_ASSERT_PTR(cJSON_AddStringToObject(fn, "description", description));
    MY_ASSERT_PTR(cJSON_AddStringToObject(fn, "name", name));

    cJSON* dup_params = NULL;
    if (parameters) {
        dup_params = cJSON_Duplicate(parameters, true);
        MY_ASSERT_PTR(dup_params);
    } else {
        dup_params = cJSON_CreateObject();
        MY_ASSERT_PTR(dup_params);
    }

    cJSON_AddItemToObject(fn, "parameters", dup_params); /* ownership transferred here */
    cJSON_AddItemToArray(api->tools, tool);
}
static MyAIAPITool* MyAIAPI_FindTool(MyAIAPI* api, const char* name) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_PTR(name);
    
    for (size_t i = 0; i < MY_AIAPI_TOOL_COUNT; ++i) {
        if (api->attached[i].in_use && strcmp(api->attached[i].name, name) == 0) {
            return &api->attached[i];
        }
    }
    return NULL;
}

/* --------------------------------------------------------------------------
 * Backends
 * -------------------------------------------------------------------------- */

    /* --------------------------------------------------------------------------
     * Openai
     * -------------------------------------------------------------------------- */

    static bool MyAIAPI_OpenaiParseArgs(cJSON* raw, cJSON** out) {
        if (!cJSON_IsString(raw)) { return false; }
        *out = cJSON_Parse(raw->valuestring);
        return *out != NULL;
    }
    static void MyAIAPI_OpenaiPushResult(MyAIAPI* api, const char* id, const char* name, cJSON* result) {
        cJSON* msg = cJSON_CreateObject();
        cJSON_AddStringToObject(msg, "role", "tool");
        cJSON_AddStringToObject(msg, "tool_call_id", id);

        char* printed = cJSON_PrintUnformatted(result);
        if (printed) {
            MY_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", printed));
            MY_FREE_IF(printed);
        } else {
            MY_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", "{}"));
        }

        MyAIAPI_HistoryInsert(api, MyAIAPI_HistorySize(api), msg);
    }
    static void MyAIAPI_OpenaiInitHeaders(MyAIAPI* api) {
        MyAIAPI_AppendHdr(&api->headers, "Authorization: Bearer %s", api->config.api_key);
        MyAIAPI_AppendHdr(&api->headers, "Content-Type: application/json");
    }

    static const struct MyAIAPIBackendOps MYAIAPI_OPENAI_BACKEND = {
        .init_headers      = MyAIAPI_OpenaiInitHeaders,
        .parse_tool_args   = MyAIAPI_OpenaiParseArgs,
        .push_tool_result  = MyAIAPI_OpenaiPushResult,
        .supports_seed     = true
    };

  /* --------------------------------------------------------------------------
   * Openrouter
   * -------------------------------------------------------------------------- */

    static bool MyAIAPI_OpenrouterParseArgs(cJSON* raw, cJSON** out) {
        if (cJSON_IsString(raw)) {
            *out = cJSON_Parse(raw->valuestring);
        } else if (cJSON_IsObject(raw)) {
            *out = cJSON_Duplicate(raw, true);
        } else { 
            return false;
        }
        return *out != NULL;
    }
    static void MyAIAPI_OpenrouterPushResult(MyAIAPI* api, const char* id, const char* name, cJSON* result) {
        cJSON* msg = cJSON_CreateObject();
        cJSON_AddStringToObject(msg, "role", "tool");
        cJSON_AddStringToObject(msg, "tool_call_id", id);
        cJSON_AddStringToObject(msg, "name", name);

        char* printed = cJSON_PrintUnformatted(result);
        if (printed) {
            MY_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", printed));
            cJSON_free(printed);
        } else {
            MY_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", "{}"));
        }

        MyAIAPI_HistoryInsert(api, MyAIAPI_HistorySize(api), msg);
    }
    static void MyAIAPI_OpenrouterInitHeaders(MyAIAPI* api) {
        MyAIAPI_AppendHdr(&api->headers, "Authorization: Bearer %s", api->config.api_key);
        MyAIAPI_AppendHdr(&api->headers, "Content-Type: application/json");

        if (api->config.http_referer) {
            MyAIAPI_AppendHdr(&api->headers, "HTTP-Referer: %s", api->config.http_referer);
        }
        if (api->config.x_title) {
            MyAIAPI_AppendHdr(&api->headers, "X-Title: %s", api->config.x_title);
        }
    }

    static const struct MyAIAPIBackendOps MYAIAPI_OPENROUTER_BACKEND = {
        .init_headers      = MyAIAPI_OpenrouterInitHeaders,
        .parse_tool_args   = MyAIAPI_OpenrouterParseArgs,
        .push_tool_result  = MyAIAPI_OpenrouterPushResult,
        .supports_seed     = true
    };

  /* --------------------------------------------------------------------------
   * Llama.cpp
   * -------------------------------------------------------------------------- */

    
    static void MyAIAPI_LlamaInitHeaders(MyAIAPI* api) {
        MyAIAPI_AppendHdr(&api->headers, "Content-Type: application/json");
    }

    static const struct MyAIAPIBackendOps MYAIAPI_LLAMA_BACKEND = {
      .init_headers      = MyAIAPI_LlamaInitHeaders,
      .parse_tool_args   = MyAIAPI_OpenaiParseArgs,  /* same as OpenAI */
      .push_tool_result  = MyAIAPI_OpenaiPushResult, /* same as OpenAI */
      .supports_seed     = false
    };

/* --------------------------------------------------------------------------
 * Implementation
 * -------------------------------------------------------------------------- */

MyAIAPI*  MyAIAPI_Create        (MyAIAPI* api, MyAIAPIConfig config) {
    MY_ASSERT_PTR(config.model);

    if (!api) {
        MY_CALLOC(api, struct MyAIAPI, 1);
        api->allocated = true;
    }

    MY_RWLOCK_INIT(api->lock);

    memset(api->attached, 0, sizeof(api->attached));
    api->config = config;

    /* ------------------------------------------------------------------ */
    /* Defaults                                                           */
    /* ------------------------------------------------------------------ */

    api->config.max_history    = config.max_history    > 0                          ? config.max_history    : MY_AIAPI_OPENAI_DEFAULT_HISTORY;
    api->config.max_remove_idx = config.max_remove_idx >= 0                         ? config.max_remove_idx : MY_AIAPI_OPENAI_DEFAULT_REMOVE_IDX;
    api->config.max_tokens     = (config.max_tokens > 0 || config.max_tokens == -1) ? config.max_tokens     : MY_AIAPI_OPENAI_DEFAULT_MAXTOKENS;
    api->config.temperature    = config.temperature >= 0.f                          ? config.temperature    : MY_AIAPI_OPENAI_DEFAULT_TEMPERATURE;
    api->config.top_p          = config.top_p >= 0.f                                ? config.top_p          : MY_AIAPI_OPENAI_DEFAULT_TOP_P;

    MY_ASSERT_BOUNDS(api->config.max_remove_idx, api->config.max_history - 2);

    /* ------------------------------------------------------------------ */
    /* Backend selection                                                  */
    /* ------------------------------------------------------------------ */

    if (config.backend == MY_AIAPI_OPENAI) {
        api->backend = &MYAIAPI_OPENAI_BACKEND;
        if (!api->config.url) { api->config.url = MY_AIAPI_OPENAI_DEFAULT_URL; }
        MY_ASSERT_PTR(api->config.api_key);
    } else if (config.backend == MY_AIAPI_OPENROUTER) {
        api->backend = &MYAIAPI_OPENROUTER_BACKEND;
        if (!api->config.url) { api->config.url = MY_AIAPI_OPENROUTER_DEFAULT_URL; }
        if (!api->config.x_title) { api->config.x_title = MY_AIAPI_OPENROUTER_DEFAULT_X_TITLE; }
        MY_ASSERT_PTR(api->config.api_key);
    } else if (config.backend == MY_AIAPI_LLAMA_CPP) {
        api->backend = &MYAIAPI_LLAMA_BACKEND;
        MY_ASSERT_PTR(api->config.url); /* llama MUST provide url */
    } else {
        MY_ASSERT(false, "Unknown MyAIAPI backend");
    }

    MY_ASSERT_PTR(api->backend);

    /* ------------------------------------------------------------------ */
    /* Root JSON                                                          */
    /* ------------------------------------------------------------------ */

    api->root = cJSON_CreateObject();
    MY_ASSERT_PTR(api->root);

    api->tools    = cJSON_AddArrayToObject(api->root, "tools");
    api->messages = cJSON_AddArrayToObject(api->root, "messages");

    cJSON_AddStringToObject(api->root, "model", api->config.model);
    cJSON_AddNumberToObject(api->root, "temperature", api->config.temperature);
    cJSON_AddNumberToObject(api->root, "top_p", api->config.top_p);

    if (api->config.max_tokens != -1) { 
        MY_ASSERT_PTR(cJSON_AddNumberToObject(api->root, "max_tokens", api->config.max_tokens));
    }

    if (api->config.seed >= 0 && api->backend->supports_seed) { 
        MY_ASSERT_PTR(cJSON_AddNumberToObject(api->root, "seed", api->config.seed));
    }

    if (api->config.reasoning != -1 && config.backend == MY_AIAPI_OPENROUTER) {
        cJSON* reasoning = cJSON_CreateObject();
        MY_ASSERT_PTR(reasoning);
        MY_ASSERT_PTR(cJSON_AddBoolToObject(reasoning, "enabled", !!api->config.reasoning));
        cJSON_AddItemToObject(api->root, "reasoning", reasoning);
    }

    if (api->config.presence_penalty != 0.f) {
        MY_ASSERT_PTR(cJSON_AddNumberToObject(api->root, "presence_penalty", api->config.presence_penalty));
    }

    if (api->config.frequency_penalty != 0.f) {
        MY_ASSERT_PTR(cJSON_AddNumberToObject(api->root, "frequency_penalty", api->config.frequency_penalty));
    }

    if (api->config.stop) {
        MY_ASSERT(cJSON_IsString(api->config.stop) || cJSON_IsArray(api->config.stop), "`stop` must be string or array");
        cJSON_AddItemToObject(api->root, "stop", cJSON_Duplicate(api->config.stop, true));
    }

    if (api->config.response_format) {
        cJSON* dup_resp = cJSON_Duplicate(api->config.response_format, true);
        MY_ASSERT_PTR(dup_resp);
        cJSON_AddItemToObject(api->root, "response_format", dup_resp);
    }

    if (api->config.logit_bias) {
        MY_ASSERT(cJSON_IsObject(api->config.logit_bias), "logit_bias must be object");
        cJSON* dup_lb = cJSON_Duplicate(api->config.logit_bias, true);
        MY_ASSERT_PTR(dup_lb);
        cJSON_AddItemToObject(api->root, "logit_bias", dup_lb);
    }

    /* ------------------------------------------------------------------ */
    /* CURL                                                               */
    /* ------------------------------------------------------------------ */

    api->curl = curl_easy_init();
    MY_ASSERT(api->curl, "curl_easy_init failed");

    api->headers = NULL;
    api->backend->init_headers(api);

    CURLcode cres = curl_easy_setopt(api->curl, CURLOPT_POST, 1L);
    MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_POST failed: %s", curl_easy_strerror(cres)));

    return api;
}
void      MyAIAPI_Destroy       (MyAIAPI* api) {
    MY_ASSERT_PTR(api);

    if (api->root)      { cJSON_Delete(api->root); }
    if (api->headers)   { curl_slist_free_all(api->headers); }
    if (api->curl)      { curl_easy_cleanup(api->curl); }

    MY_FREE_IF(api->response);

    MY_RWLOCK_DESTROY(api->lock);

    if (api->allocated) {
        MY_FREE(api);
    }
}

cJSON*    MyAIAPI_Send          (MyAIAPI* api, cJSON* message) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_PTR(message);
    MY_ASSERT_PTR(api->backend);
    MY_ASSERT_PTR(api->curl);
    MY_ASSERT_PTR(api->root);
    MY_ASSERT_PTR(api->messages);

    MyAIAPI_HistoryInsert(api, MyAIAPI_HistorySize(api), cJSON_Duplicate(message, true));

    while (true) {
        MY_FREE_IF(api->response);
        api->response_size = 0;

        char* payload = cJSON_PrintUnformatted(api->root);
        MY_ASSERT_PTR(payload);

        CURLcode cres;
        cres = curl_easy_setopt(api->curl, CURLOPT_URL, api->config.url);
        MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_URL failed: %s", curl_easy_strerror(cres)));
        cres = curl_easy_setopt(api->curl, CURLOPT_HTTPHEADER, api->headers);
        MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_HTTPHEADER failed: %s", curl_easy_strerror(cres)));
        cres = curl_easy_setopt(api->curl, CURLOPT_POSTFIELDS, payload);
        MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_POSTFIELDS failed: %s", curl_easy_strerror(cres)));
        cres = curl_easy_setopt(api->curl, CURLOPT_WRITEFUNCTION, MyAIAPI_WriteCallback);
        MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_WRITEFUNCTION failed: %s", curl_easy_strerror(cres)));
        cres = curl_easy_setopt(api->curl, CURLOPT_WRITEDATA, api);
        MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_WRITEDATA failed: %s", curl_easy_strerror(cres)));

        if (api->config.timeout_ms > 0) {
            cres = curl_easy_setopt(api->curl, CURLOPT_TIMEOUT_MS, api->config.timeout_ms);
            MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_TIMEOUT_MS failed: %s", curl_easy_strerror(cres)));
        }

        if (api->config.cainfo_path) {
            cres = curl_easy_setopt(api->curl, CURLOPT_CAINFO, api->config.cainfo_path);
            MY_ASSERT(cres == CURLE_OK, MySprintf("curl_easy_setopt CURLOPT_CAINFO failed: %s", curl_easy_strerror(cres)));
        }

        CURLcode res = curl_easy_perform(api->curl);
        MY_FREE(payload);

        MY_ASSERT(res == CURLE_OK, MySprintf("curl_easy_perform failed: %s", curl_easy_strerror(res)));
        MY_ASSERT(api->response_size > 0, "Empty HTTP response");

        cJSON* root = cJSON_Parse(api->response);
        MY_ASSERT_PTR(root);

        /* -------------------------------------------------------------- */
        /* ERROR                                                          */
        /* -------------------------------------------------------------- */

        cJSON* err = cJSON_GetObjectItem(root, "error");
        if (err) {
            char* e = cJSON_Print(err);
            char buffer[1024] = {0};
            strncpy(buffer, e ? e : "<unprintable>", sizeof(buffer));
            cJSON_free(e);
            MY_ASSERT(false, MySprintf("API error: %s\n", buffer));
        }

        /* -------------------------------------------------------------- */
        /* CHOICES                                                        */
        /* -------------------------------------------------------------- */

        cJSON* choices = cJSON_GetObjectItem(root, "choices");
        MY_ASSERT(choices && cJSON_IsArray(choices), MySprintf("Missing choices\n%s", api->response));

        cJSON* choice = cJSON_GetArrayItem(choices, 0);
        MY_ASSERT_PTR(choice);

        /* -------------------------------------------------------------- */
        /* MESSAGE (OpenAI / OpenRouter / llama.cpp compatible)           */
        /* -------------------------------------------------------------- */

        bool llama_msg = false;
        cJSON* msg = cJSON_GetObjectItem(choice, "message");

        if (!msg) {
            /* llama.cpp fallback */
            cJSON* text = cJSON_GetObjectItem(choice, "text");
            MY_ASSERT(text && cJSON_IsString(text), "Invalid llama.cpp response");

            msg = cJSON_CreateObject();
            MY_ASSERT_PTR(msg);
            MY_ASSERT_PTR(cJSON_AddStringToObject(msg, "role", "assistant"));
            MY_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", text->valuestring));
            llama_msg = true;
        }

        MY_ASSERT_PTR(msg);

        /* -------------------------------------------------------------- */
        /* TOOL CALLS                                                     */
        /* -------------------------------------------------------------- */

        cJSON* tool_calls = cJSON_GetObjectItem(msg, "tool_calls");

        if (tool_calls && cJSON_IsArray(tool_calls)) {
            cJSON* dup_msg = cJSON_Duplicate(msg, true);
            MY_ASSERT_PTR(dup_msg);
            MyAIAPI_HistoryInsert(api, MyAIAPI_HistorySize(api), dup_msg);

            cJSON* call;
            cJSON_ArrayForEach(call, tool_calls) {
                cJSON* id = cJSON_GetObjectItem(call, "id");
                cJSON* fn = cJSON_GetObjectItem(call, "function");
                MY_ASSERT_PTR(fn);

                cJSON* name = cJSON_GetObjectItem(fn, "name");
                cJSON* args_raw = cJSON_GetObjectItem(fn, "arguments");
                MY_ASSERT_PTR(name);
                MY_ASSERT_PTR(args_raw);

                cJSON* args = NULL;
                MY_ASSERT(api->backend->parse_tool_args(args_raw, &args), "Failed to parse tool arguments");

                MyAIAPITool* tool = MyAIAPI_FindTool(api, name->valuestring);
                MY_ASSERT(tool != NULL, MySprintf("Unknown tool requested: %s", name->valuestring));
                MY_ASSERT_PTR(tool->fn);
                MY_ASSERT_PTR(tool->parameters); /* tool should have parameters object */

                cJSON* result = tool->fn(args);
                cJSON_Delete(args);
                MY_ASSERT_PTR(result);

                api->backend->push_tool_result(api, id ? id->valuestring : "", name->valuestring, result);
                cJSON_Delete(result);
            }

            cJSON_Delete(root);
            continue; /* tool loop */
        }

        /* -------------------------------------------------------------- */
        /* FINAL MESSAGE                                                  */
        /* -------------------------------------------------------------- */

        cJSON* response = cJSON_Duplicate(msg, true);
        MY_ASSERT_PTR(response);
        MyAIAPI_HistoryInsert(api, MyAIAPI_HistorySize(api), response);

        if (llama_msg) { cJSON_free(msg); }
        cJSON_Delete(root);
        return response;
    }
}

void      MyAIAPI_Rdlock        (MyAIAPI* api) {
    MY_ASSERT_PTR(api);
    MY_RWLOCK_RDLOCK(api->lock);
}
void      MyAIAPI_Wrlock        (MyAIAPI* api) {
    MY_ASSERT_PTR(api);
    MY_RWLOCK_WRLOCK(api->lock);
}
void      MyAIAPI_Rdunlock      (MyAIAPI* api) {
    MY_ASSERT_PTR(api);
    MY_RWLOCK_RDUNLOCK(api->lock);
}
void      MyAIAPI_Wrunlock      (MyAIAPI* api) {
    MY_ASSERT_PTR(api);
    MY_RWLOCK_WRUNLOCK(api->lock);
}

void      MyAIAPI_ToolCreate    (MyAIAPITool* tool, const char* name, const char* description, MyAIAPIToolFn fn) {
    MY_ASSERT_PTR(tool);
    MY_ASSERT_PTR(name);
    MY_ASSERT_PTR(description);
    MY_ASSERT_PTR(fn);

    tool->fn = fn;
    tool->name = name;
    tool->description = description;
    tool->parameters = cJSON_CreateObject();
    MY_ASSERT_PTR(tool->parameters);

    cJSON* properties = cJSON_AddObjectToObject(tool->parameters, "properties");
    MY_ASSERT_PTR(properties);
    MY_ASSERT_PTR(cJSON_AddStringToObject(tool->parameters, "type", "object"));
}
void      MyAIAPI_ToolAddParam  (MyAIAPITool* tool, const char* name, const char* type) {
    MY_ASSERT_PTR(tool);
    MY_ASSERT_PTR(name);
    MY_ASSERT_PTR(type);
    MY_ASSERT_PTR(tool->parameters);

    cJSON* properties = cJSON_GetObjectItem(tool->parameters, "properties");
    MY_ASSERT_PTR(properties);

    cJSON* parameter = cJSON_AddObjectToObject(properties, name);
    MY_ASSERT_PTR(parameter);
    MY_ASSERT_PTR(cJSON_AddStringToObject(parameter, "type", type));
}
void      MyAIAPI_ToolAttach    (MyAIAPI* api, size_t idx, MyAIAPITool tool) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_BOUNDS(idx, MY_AIAPI_TOOL_COUNT);
    MY_ASSERT(!api->attached[idx].in_use, "Trying to attach a tool in an already attached slot");

    MY_ASSERT_PTR(tool.fn);
    MY_ASSERT_PTR(tool.name);
    MY_ASSERT_PTR(tool.parameters);

    tool.in_use = true;
    if (tool.description == NULL) {
        tool.description = "No description provided";
    }

    for (size_t i = 0; i < MY_AIAPI_TOOL_COUNT; ++i) {
        if (api->attached[i].in_use) {
            MY_ASSERT(strcmp(api->attached[i].name, tool.name) != 0, MySprintf("Tool name already attached: %s", tool.name));
        }
    }

    MyAIAPI_PushTool(api, tool.name, tool.description, tool.parameters);
    api->attached[idx] = tool;
}
void      MyAIAPI_ToolDetach    (MyAIAPI* api, size_t idx) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_BOUNDS(idx, MY_AIAPI_TOOL_COUNT);
    MY_ASSERT(api->attached[idx].in_use, "Trying to deattach a tool in a free slot");

    cJSON_DeleteItemFromObject(api->root, "tools");
    api->tools = cJSON_AddArrayToObject(api->root, "tools");
    MY_ASSERT_PTR(api->tools);
    memset(&api->attached[idx], 0, sizeof(MyAIAPITool));

    for (size_t i = 0; i < MY_AIAPI_TOOL_COUNT; i++) {
        if (api->attached[i].in_use) {
            MyAIAPI_PushTool(api, api->attached[i].name, api->attached[i].description, api->attached[i].parameters);
        }
    }
}

void      MyAIAPI_HistorySet    (MyAIAPI* api, size_t idx, cJSON* message) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_PTR(message);
    MY_ASSERT_PTR(api->messages);
    MY_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages));
    cJSON_ReplaceItemInArray(api->messages, idx, message);
}
cJSON*    MyAIAPI_HistoryGet    (MyAIAPI* api, size_t idx) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_PTR(api->messages);
    MY_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages));
    return cJSON_GetArrayItem(api->messages, idx);
}
void      MyAIAPI_HistoryErase  (MyAIAPI* api, size_t idx) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_PTR(api->messages);
    MY_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages));
    cJSON_DeleteItemFromArray(api->messages, idx);
}
void      MyAIAPI_HistoryInsert (MyAIAPI* api, size_t idx, cJSON* message) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_PTR(message);
    MY_ASSERT_PTR(api->messages);
    MY_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages) + 1);

    cJSON_InsertItemInArray(api->messages, idx, message);

    while (cJSON_GetArraySize(api->messages) > api->config.max_history) {
        cJSON_DeleteItemFromArray(api->messages, api->config.max_remove_idx);
    }
}
int       MyAIAPI_HistorySize   (MyAIAPI* api) {
    MY_ASSERT_PTR(api);
    MY_ASSERT_PTR(api->messages);
    return cJSON_GetArraySize(api->messages);
}