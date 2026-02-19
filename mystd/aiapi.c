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

static size_t MyAIAPI_WriteCallback(void* data, size_t size, size_t nmemb, void* userp) {
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
static void   MyAIAPI_AppendHdr(struct curl_slist** header, const char* format, ...) {
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
static void   MyAIAPI_PushTool(MyAIAPI* api, const char* name, const char* description, const cJSON* parameters) {
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

static const MyAIAPIBackendOps* openai_ops = NULL;

/* --------------------------------------------------------------------------
 * Openai
 * -------------------------------------------------------------------------- */

    static bool MyAIAPI_OpenaiParseArgs(cJSON* raw, cJSON** out) {
        if (!cJSON_IsString(raw)) { return 0; }
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

    static bool MyAIAPI_OpenaiSupportsSeed(void) { return true; }

    static const struct MyAIAPIBackendOps MYAIAPI_OPENAI_BACKEND = {
        .init_request      = NULL,
        .init_headers      = MyAIAPI_OpenaiInitHeaders,
        .parse_tool_args   = MyAIAPI_OpenaiParseArgs,
        .push_tool_result  = MyAIAPI_OpenaiPushResult,
        .supports_seed     = MyAIAPI_OpenaiSupportsSeed
    };

  /* --------------------------------------------------------------------------
   * Openrouter
   * -------------------------------------------------------------------------- */

static int openrouter_parse_args(cJSON* raw, cJSON** out) {
  if (cJSON_IsString(raw)) {
    *out = cJSON_Parse(raw->valuestring);
  } else if (cJSON_IsObject(raw)) {
    *out = cJSON_Duplicate(raw, true);
  } else return 0;
  return *out != NULL;
}

static void openrouter_push_result(MyAIAPI* api, const char* id, const char* name, cJSON* result) {
  cJSON* msg = cJSON_CreateObject();
  cJSON_AddStringToObject(msg, "role", "tool");
  cJSON_AddStringToObject(msg, "tool_call_id", id);
  cJSON_AddStringToObject(msg, "name", name);

  char* printed = cJSON_PrintUnformatted(result);
  if (printed) {
    MY_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", printed));
    MY_FREE_IF(printed);
  } else {
    MY_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", "{}"));
  }

  fn_openai_history_insert(api, fn_openai_history_size(api), msg);
}

static void openrouter_init_headers(MyAIAPI* api) {
  openai_append_hdr(&api->headers, "Authorization: Bearer %s", api->config.api_key);
  openai_append_hdr(&api->headers, "Content-Type: application/json");

  if (api->config.http_referer) {
    openai_append_hdr(&api->headers, "HTTP-Referer: %s", api->config.http_referer);
  }
  if (api->config.x_title) {
    openai_append_hdr(&api->headers, "X-Title: %s", api->config.x_title);
  }
}

static int openrouter_supports_seed(void) { return 1; }

static const struct MyAIAPI_backend_ops OPENROUTER_BACKEND = {
  .init_request      = NULL,
  .init_headers      = openrouter_init_headers,
  .parse_tool_args   = openrouter_parse_args,
  .push_tool_result  = openrouter_push_result,
  .supports_seed     = openrouter_supports_seed
};

  /* --------------------------------------------------------------------------
   * Llama.cpp
   * -------------------------------------------------------------------------- */

static int llama_parse_args(cJSON* raw, cJSON** out) {
  if (!cJSON_IsString(raw)) { return 0; }
  *out = cJSON_Parse(raw->valuestring);
  return *out != NULL;
}

static void llama_init_headers(MyAIAPI* api) {
  openai_append_hdr(&api->headers, "Content-Type: application/json");
}

static int llama_supports_seed(void) { return 0; }

static const struct MyAIAPI_backend_ops LLAMA_BACKEND = {
  .init_request      = NULL,
  .init_headers      = llama_init_headers,
  .parse_tool_args   = llama_parse_args,
  .push_tool_result  = openai_push_result, /* same as OpenAI */
  .supports_seed     = llama_supports_seed
};

/* --------------------------------------------------------------------------
 * Implementation
 * -------------------------------------------------------------------------- */

MyAIAPI*  fn_openai_create(MyAIAPI* api, MyAIAPI_config config) {
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

  api->config.max_history    = config.max_history    > 0                          ? config.max_history    : OPENAI_DEFAULT_HISTORY;
  api->config.max_remove_idx = config.max_remove_idx >= 0                         ? config.max_remove_idx : OPENAI_DEFAULT_REMOVE_IDX;
  api->config.max_tokens     = (config.max_tokens > 0 || config.max_tokens == -1) ? config.max_tokens     : OPENAI_DEFAULT_MAXTOKENS;
  api->config.temperature    = config.temperature >= 0.f                          ? config.temperature    : OPENAI_DEFAULT_TEMPERATURE;
  api->config.top_p          = config.top_p >= 0.f                                ? config.top_p          : OPENAI_DEFAULT_TOP_P;

  MY_ASSERT_BOUNDS(api->config.max_remove_idx, api->config.max_history - 2);

  /* ------------------------------------------------------------------ */
  /* Backend selection                                                  */
  /* ------------------------------------------------------------------ */

  switch (config.backend) {
    case OPENAI_API:
      openai_ops = &OPENAI_BACKEND;
      if (!api->config.url) { api->config.url = OPENAI_DEFAULT_URL; }
      MY_ASSERT_PTR(api->config.api_key);
      break;

    case OPENROUTER_API:
      openai_ops = &OPENROUTER_BACKEND;
      if (!api->config.url) { api->config.url = OPENROUTER_DEFAULT_URL; }
      if (!api->config.x_title) { api->config.x_title = OPENROUTER_DEFAULT_X_TITLE; }
      MY_ASSERT_PTR(api->config.api_key);
      break;

    case LLAMA_CPP_API:
      openai_ops = &LLAMA_BACKEND;
      /* llama MUST provide url */
      MY_ASSERT_PTR(api->config.url);
      break;

    default:
      MY_ASSERT(0, "Unknown backend");
  }

  MY_ASSERT_PTR(openai_ops);

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
  
  if (api->config.seed >= 0 && openai_ops->supports_seed()) { 
    MY_ASSERT_PTR(cJSON_AddNumberToObject(api->root, "seed", api->config.seed));
  }

  if (api->config.reasoning != -1 && config.backend == OPENROUTER_API) {
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
    MY_ASSERT(
      cJSON_IsString(api->config.stop) || cJSON_IsArray(api->config.stop),
      "`stop` must be string or array"
    );
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
  openai_ops->init_headers(api);

  CURLcode cres = curl_easy_setopt(api->curl, CURLOPT_POST, 1L);
  MY_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_POST failed: %s", curl_easy_strerror(cres));

  return api;
}

void            fn_openai_destroy(MyAIAPI* api) {
  MY_ASSERT_PTR(api);

  if (api->root) { cJSON_Delete(api->root); }
  if (api->headers) { curl_slist_free_all(api->headers); }
  if (api->curl) { curl_easy_cleanup(api->curl); }

  MY_FREE_IF(api->response);

  MY_RWLOCK_DESTROY(api->lock);

  if (api->allocated) {
    MY_FREE(api);
  }
}

cJSON*          fn_openai_send(MyAIAPI* api, cJSON* message) {
  MY_ASSERT_PTR(api);
  MY_ASSERT_PTR(message);
  MY_ASSERT_PTR(openai_ops);
  MY_ASSERT_PTR(api->curl);
  MY_ASSERT_PTR(api->root);
  MY_ASSERT_PTR(api->messages);

  fn_openai_history_insert(
    api,
    fn_openai_history_size(api),
    cJSON_Duplicate(message, true)
  );

  while (true) {
    MY_FREE_IF(api->response);
    api->response_size = 0;

    char* payload = cJSON_PrintUnformatted(api->root);
    MY_ASSERT_PTR(payload);

    CURLcode cres;
    cres = curl_easy_setopt(api->curl, CURLOPT_URL, api->config.url);
    MY_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_URL failed: %s", curl_easy_strerror(cres));
    cres = curl_easy_setopt(api->curl, CURLOPT_HTTPHEADER, api->headers);
    MY_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_HTTPHEADER failed: %s", curl_easy_strerror(cres));
    cres = curl_easy_setopt(api->curl, CURLOPT_POSTFIELDS, payload);
    MY_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_POSTFIELDS failed: %s", curl_easy_strerror(cres));
    cres = curl_easy_setopt(api->curl, CURLOPT_WRITEFUNCTION, openai_write_callback);
    MY_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_WRITEFUNCTION failed: %s", curl_easy_strerror(cres));
    cres = curl_easy_setopt(api->curl, CURLOPT_WRITEDATA, api);
    MY_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_WRITEDATA failed: %s", curl_easy_strerror(cres));

    if (api->config.timeout_ms > 0) {
      cres = curl_easy_setopt(api->curl, CURLOPT_TIMEOUT_MS, api->config.timeout_ms);
      MY_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_TIMEOUT_MS failed: %s", curl_easy_strerror(cres));
    }

    if (api->config.cainfo_path) {
      cres = curl_easy_setopt(api->curl, CURLOPT_CAINFO, api->config.cainfo_path);
      MY_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_CAINFO failed: %s", curl_easy_strerror(cres));
    }

    CURLcode res = curl_easy_perform(api->curl);
    MY_FREE(payload);

    MY_ASSERTF(res == CURLE_OK, "curl_easy_perform failed: %s", curl_easy_strerror(res));
    MY_ASSERT(api->response_size > 0, "Empty HTTP response");

    cJSON* root = cJSON_Parse(api->response);
    MY_ASSERT_PTR(root);

    /* -------------------------------------------------------------- */
    /* ERROR                                                          */
    /* -------------------------------------------------------------- */

    cJSON* err = cJSON_GetObjectItem(root, "error");
    if (err) {
      char* e = cJSON_Print(err);
      MY_ASSERTF(false, "API error:\n%s", e ? e : "<unprintable>");
    }

    /* -------------------------------------------------------------- */
    /* CHOICES                                                        */
    /* -------------------------------------------------------------- */

    cJSON* choices = cJSON_GetObjectItem(root, "choices");
    MY_ASSERTF(choices && cJSON_IsArray(choices), "Missing choices\n%s", api->response);

    cJSON* choice = cJSON_GetArrayItem(choices, 0);
    MY_ASSERT_PTR(choice);

    /* -------------------------------------------------------------- */
    /* MESSAGE (OpenAI / OpenRouter / llama.cpp compatible)           */
    /* -------------------------------------------------------------- */

    cJSON* msg = cJSON_GetObjectItem(choice, "message");

    if (!msg) {
      /* llama.cpp fallback */
      cJSON* text = cJSON_GetObjectItem(choice, "text");
      MY_ASSERT(text && cJSON_IsString(text), "Invalid llama.cpp response");

      msg = cJSON_CreateObject();
      MY_ASSERT_PTR(msg);
      MY_ASSERT_PTR(cJSON_AddStringToObject(msg, "role", "assistant"));
      MY_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", text->valuestring));
    }

    MY_ASSERT_PTR(msg);

    /* -------------------------------------------------------------- */
    /* TOOL CALLS                                                     */
    /* -------------------------------------------------------------- */

    cJSON* tool_calls = cJSON_GetObjectItem(msg, "tool_calls");

    if (tool_calls && cJSON_IsArray(tool_calls)) {
      cJSON* dup_msg = cJSON_Duplicate(msg, true);
      MY_ASSERT_PTR(dup_msg);
      fn_openai_history_insert(api, fn_openai_history_size(api), dup_msg);

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

        if (cJSON_IsString(args_raw)) {
          args = cJSON_Parse(args_raw->valuestring);
        } else if (cJSON_IsObject(args_raw)) {
          args = cJSON_Duplicate(args_raw, true);
        }

        MY_ASSERT(args != NULL, "Failed to parse tool arguments");

        MyAIAPI_tool* tool = openai_find_tool(api, name->valuestring);
        MY_ASSERTF(tool != NULL, "Unknown tool requested: %s", name->valuestring);
        MY_ASSERT_PTR(tool->fn);
        MY_ASSERT_PTR(tool->parameters); /* tool should have parameters object */

        cJSON* result = tool->fn(args);
        cJSON_Delete(args);
        MY_ASSERT_PTR(result);

        openai_ops->push_tool_result(api, id ? id->valuestring : "", name->valuestring, result);
        cJSON_Delete(result);
      }

      cJSON_Delete(root);
      continue; /* 🔁 tool loop */
    }

    /* -------------------------------------------------------------- */
    /* FINAL MESSAGE                                                  */
    /* -------------------------------------------------------------- */

    cJSON* response = cJSON_Duplicate(msg, true);
    MY_ASSERT_PTR(response);
    fn_openai_history_insert(api, fn_openai_history_size(api), response);

    cJSON_Delete(root);
    return response;
  }
}

void            fn_openai_rdlock(MyAIAPI* api) {
  MY_ASSERT_PTR(api);
  MY_RWLOCK_RDLOCK(api->lock);
}

void            fn_openai_wrlock(MyAIAPI* api) {
  MY_ASSERT_PTR(api);
  MY_RWLOCK_WRLOCK(api->lock);
}

void            fn_openai_rdunlock(MyAIAPI* api) {
  MY_ASSERT_PTR(api);
  MY_RWLOCK_RDUNLOCK(api->lock);
}

void            fn_openai_wrunlock(MyAIAPI* api) {
  MY_ASSERT_PTR(api);
  MY_RWLOCK_WRUNLOCK(api->lock);
}

void            fn_openai_tool_create(MyAIAPI_tool* tool, const char* name, const char* description, type_openai_tool_fn fn) {
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

void            fn_openai_tool_add_param(MyAIAPI_tool* tool, const char* name, const char* type) {
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

void            fn_openai_tool_attach(MyAIAPI* api, size_t idx, MyAIAPI_tool tool) {
  MY_ASSERT_PTR(api);
  MY_ASSERT_BOUNDS(idx, OPENAI_TOOL_COUNT);
  MY_ASSERT(!api->attached[idx].in_use, "Trying to attach a tool in an already attached slot");

  MY_ASSERT_PTR(tool.fn);
  MY_ASSERT_PTR(tool.name);
  MY_ASSERT_PTR(tool.parameters);

  tool.in_use = true;
  if (tool.description == NULL) {
    tool.description = "No description provided";
  }

  for (size_t i = 0; i < OPENAI_TOOL_COUNT; ++i) {
    if (api->attached[i].in_use) {
      MY_ASSERTF(strcmp(api->attached[i].name, tool.name) != 0, "Tool name already attached: %s", tool.name);
    }
  }

  openai_push_tool(api, tool.name, tool.description, tool.parameters);
  api->attached[idx] = tool;
}

void            fn_openai_tool_deattach(MyAIAPI* api, size_t idx) {
  MY_ASSERT_PTR(api);
  MY_ASSERT_BOUNDS(idx, OPENAI_TOOL_COUNT);
  MY_ASSERT_FORCED(api->attached[idx].in_use, "Trying to deattach a tool in a free slot");

  cJSON_DeleteItemFromObject(api->root, "tools");
  api->tools = cJSON_AddArrayToObject(api->root, "tools");
  MY_ASSERT_PTR(api->tools);
  memset(&api->attached[idx], 0, sizeof(MyAIAPI_tool));

  for (size_t i = 0; i < OPENAI_TOOL_COUNT; i++) {
    if (api->attached[i].in_use) {
      openai_push_tool(api, api->attached[i].name, api->attached[i].description, api->attached[i].parameters);
    }
  }
}

void            fn_openai_history_set(MyAIAPI* api, size_t idx, cJSON* message) {
  MY_ASSERT_PTR(api);
  MY_ASSERT_PTR(message);
  MY_ASSERT_PTR(api->messages);
  MY_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages));
  cJSON_ReplaceItemInArray(api->messages, idx, message);
}

cJSON*          fn_openai_history_get(MyAIAPI* api, size_t idx) {
  MY_ASSERT_PTR(api);
  MY_ASSERT_PTR(api->messages);
  MY_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages));
  return cJSON_GetArrayItem(api->messages, idx);
}

void            fn_openai_history_erase(MyAIAPI* api, size_t idx) {
  MY_ASSERT_PTR(api);
  MY_ASSERT_PTR(api->messages);
  MY_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages));
  cJSON_DeleteItemFromArray(api->messages, idx);
}

void            fn_openai_history_insert(MyAIAPI* api, size_t idx, cJSON* message) {
  MY_ASSERT_PTR(api);
  MY_ASSERT_PTR(message);
  MY_ASSERT_PTR(api->messages);
  MY_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages) + 1);

  cJSON_InsertItemInArray(api->messages, idx, message);

  while (cJSON_GetArraySize(api->messages) > api->config.max_history) {
    cJSON_DeleteItemFromArray(api->messages, api->config.max_remove_idx);
  }
}

int             fn_openai_history_size(MyAIAPI* api) {
  MY_ASSERT_PTR(api);
  MY_ASSERT_PTR(api->messages);
  return cJSON_GetArraySize(api->messages);
}