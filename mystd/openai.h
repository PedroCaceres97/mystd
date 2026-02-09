#ifndef __MYSTD_OPENAI_H__
#define __MYSTD_OPENAI_H__

#include <mystd/stdlib.h>
#include <curl/curl.h>
#include <cJSON/cJSON.h>

#ifndef MY_OPENAI_NAME
  #define MY_OPENAI_NAME MyOpenAI
#endif /* MY_OPENAI_NAME */

#ifndef MY_OPENAI_FN_PREFIX
  #define MY_OPENAI_FN_PREFIX MY_OPENAI_NAME
#endif /* MY_OPENAI_FN_PREFIX */

#ifndef MY_OPENAI_TOOL_COUNT
  #define MY_OPENAI_TOOL_COUNT 32
#endif /* MY_OPENAI_TOOL_COUNT */

/** @cond doxygen_ignore */
#define MY_OPENAI_STRUCT             MY_OPENAI_NAME
#define MY_OPENAI_TOOL_STRUCT        MY_CONCAT2(MY_OPENAI_NAME, Tool)
#define MY_OPENAI_CONFIG_STRUCT      MY_CONCAT2(MY_OPENAI_NAME, Config)
#define MY_OPENAI_BACKEND_OPS_STRUCT MY_CONCAT2(MY_OPENAI_NAME, BackendOps)

#define MY_OPENAI_BACKEND_ENUM       MY_CONCAT2(MY_OPENAI_NAME, Backend)

#define MY_OPENAI_TOOL_FN_TYPE       MY_CONCAT2(MY_OPENAI_NAME, ToolFn)

#define MY_OPENAI_FN_CREATE          MY_CONCAT2(MY_OPENAI_FN_PREFIX, _create)
#define MY_OPENAI_FN_DESTROY         MY_CONCAT2(MY_OPENAI_FN_PREFIX, _destroy)
#define MY_OPENAI_FN_SEND            MY_CONCAT2(MY_OPENAI_FN_PREFIX, _send)

#define MY_OPENAI_FN_RDLOCK          MY_CONCAT2(MY_OPENAI_FN_PREFIX, _rdlock)
#define MY_OPENAI_FN_WRLOCK          MY_CONCAT2(MY_OPENAI_FN_PREFIX, _wrlock)
#define MY_OPENAI_FN_RDUNLOCK        MY_CONCAT2(MY_OPENAI_FN_PREFIX, _rdunlock)
#define MY_OPENAI_FN_WRUNLOCK        MY_CONCAT2(MY_OPENAI_FN_PREFIX, _wrunlock)

#define MY_OPENAI_FN_TOOL_CREATE     MY_CONCAT2(MY_OPENAI_FN_PREFIX, _tool_create)
#define MY_OPENAI_FN_TOOL_ADD_PARAM  MY_CONCAT2(MY_OPENAI_FN_PREFIX, _tool_add_param)
#define MY_OPENAI_FN_TOOL_ATTACH     MY_CONCAT2(MY_OPENAI_FN_PREFIX, _tool_attach)
#define MY_OPENAI_FN_TOOL_DETACH     MY_CONCAT2(MY_OPENAI_FN_PREFIX, _tool_detach)

#define MY_OPENAI_FN_HISTORY_SET     MY_CONCAT2(MY_OPENAI_FN_PREFIX, _history_set)
#define MY_OPENAI_FN_HISTORY_GET     MY_CONCAT2(MY_OPENAI_FN_PREFIX, _history_get)
#define MY_OPENAI_FN_HISTORY_ERASE   MY_CONCAT2(MY_OPENAI_FN_PREFIX, _history_erase)
#define MY_OPENAI_FN_HISTORY_INSERT  MY_CONCAT2(MY_OPENAI_FN_PREFIX, _history_insert)
#define MY_OPENAI_FN_HISTORY_SIZE    MY_CONCAT2(MY_OPENAI_FN_PREFIX, _history_size)
/** @endcond */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum MY_OPENAI_BACKEND_ENUM {
  MY_OPENAI_API,
  MY_OPENROUTER_API,
  MY_LLAMA_CPP_API
} MY_OPENAI_BACKEND_ENUM;

struct MY_OPENAI_STRUCT;
struct MY_OPENAI_TOOL_STRUCT;
struct MY_OPENAI_CONFIG_STRUCT;
struct MY_OPENAI_BACKEND_OPS_STRUCT;

typedef struct MY_OPENAI_STRUCT MY_OPENAI_STRUCT;
typedef struct MY_OPENAI_TOOL_STRUCT MY_OPENAI_TOOL_STRUCT;
typedef struct MY_OPENAI_CONFIG_STRUCT MY_OPENAI_CONFIG_STRUCT;
typedef struct MY_OPENAI_BACKEND_OPS_STRUCT MY_OPENAI_BACKEND_OPS_STRUCT;

/* 
 * Tool function must return a newly allocated cJSON*.
 * Ownership is transferred to Openai and it will be deleted.
 * Do not cJSON_Delete(args)
 */
typedef cJSON* (*MY_OPENAI_TOOL_FN_TYPE)(cJSON* args);

MY_OPENAI_STRUCT*      MY_OPENAI_FN_CREATE          (MY_OPENAI_STRUCT* api, MY_OPENAI_CONFIG_STRUCT config);
void                   MY_OPENAI_FN_DESTROY         (MY_OPENAI_STRUCT* api);
cJSON*                 MY_OPENAI_FN_SEND            (MY_OPENAI_STRUCT* api, cJSON* message);

void                   MY_OPENAI_FN_RDLOCK          (MY_OPENAI_STRUCT* api);
void                   MY_OPENAI_FN_WRLOCK          (MY_OPENAI_STRUCT* api);
void                   MY_OPENAI_FN_RDUNLOCK        (MY_OPENAI_STRUCT* api);
void                   MY_OPENAI_FN_WRUNLOCK        (MY_OPENAI_STRUCT* api);

void                   MY_OPENAI_FN_TOOL_CREATE     (MY_OPENAI_TOOL_STRUCT* tool, const char* name, const char* description, MY_OPENAI_TOOL_FN_TYPE fn);
void                   MY_OPENAI_FN_TOOL_ADD_PARAM  (MY_OPENAI_TOOL_STRUCT* tool, const char* name, const char* type);
void                   MY_OPENAI_FN_TOOL_ATTACH     (MY_OPENAI_STRUCT* api, size_t idx, MY_OPENAI_TOOL_STRUCT tool);
void                   MY_OPENAI_FN_TOOL_DETACH     (MY_OPENAI_STRUCT* api, size_t idx);

void                   MY_OPENAI_FN_HISTORY_SET     (MY_OPENAI_STRUCT* api, size_t idx, cJSON* message);
cJSON*                 MY_OPENAI_FN_HISTORY_GET     (MY_OPENAI_STRUCT* api, size_t idx);
void                   MY_OPENAI_FN_HISTORY_ERASE   (MY_OPENAI_STRUCT* api, size_t idx);
void                   MY_OPENAI_FN_HISTORY_INSERT  (MY_OPENAI_STRUCT* api, size_t idx, cJSON* message);
int                    MY_OPENAI_FN_HISTORY_SIZE    (MY_OPENAI_STRUCT* api);

struct MY_OPENAI_BACKEND_OPS_STRUCT {
    void (*init_request)(MY_OPENAI_STRUCT*);
    void (*init_headers)(MY_OPENAI_STRUCT*);
    int  (*parse_tool_args)(cJSON* raw, cJSON** out);
    void (*push_tool_result)(MY_OPENAI_STRUCT*, const char*, const char*, cJSON*);
    int  (*supports_seed)(void);
};

struct MY_OPENAI_CONFIG_STRUCT {
    MY_OPENAI_BACKEND_ENUM backend;

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

struct MY_OPENAI_TOOL_STRUCT {
  int         in_use;
  const char* name;
  const char* description;
  cJSON*      parameters;
  MY_OPENAI_TOOL_FN_TYPE fn;
};

struct MY_OPENAI_STRUCT {
  int allocated;
  MY_RWLOCK_TYPE lock;

  cJSON* root;
  cJSON* tools;
  cJSON* messages;

  CURL* curl;
  struct curl_slist* headers;

  char* response;
  size_t response_size;

  MY_OPENAI_TOOL_STRUCT attached[MY_OPENAI_TOOL_COUNT];
  MY_OPENAI_CONFIG_STRUCT config;
};

#ifdef MY_OPENAI_IMPLEMENTATION

/* --------------------------------------------------------------------------
 * Default macros
 * -------------------------------------------------------------------------- */

#define MY_OPENAI_DEFAULT_URL          "https://api.openai.com/v1/chat/completions"
#define MY_OPENAI_DEFAULT_HISTORY      15
#define MY_OPENAI_DEFAULT_REMOVE_IDX   1
#define MY_OPENAI_DEFAULT_MAXTOKENS    200
#define MY_OPENAI_DEFAULT_TEMPERATURE  0.5f
#define MY_OPENAI_DEFAULT_TOP_P        0.5f

#define MY_OPENROUTER_DEFAULT_URL      "https://openrouter.ai/api/v1/chat/completions"
#define MY_OPENROUTER_DEFAULT_X_TITLE  "Blop default x-title"

/* --------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------- */

static size_t openai_write_callback(void* data, size_t size, size_t nmemb, void* userp) {
  BLOP_ASSERT_PTR(data);
  BLOP_ASSERT_PTR(userp);

  struct_openai* api = (struct_openai*)userp;

  size_t total = size * nmemb;
  
  BLOP_REALLOC(api->response, char, api->response, api->response_size + total + 1);
  memcpy(api->response + api->response_size, data, total);
  api->response_size += total;
  api->response[api->response_size] = '\0';

  return total;
}

static void   openai_append_hdr(struct curl_slist** header, const char* format, ...) {
  BLOP_ASSERT_PTR(header);
  BLOP_ASSERT_PTR(format);

  char openai_temp_buffer[1024] = {0};
  va_list args;
  va_start(args, format);
  vsnprintf(openai_temp_buffer, sizeof(openai_temp_buffer), format, args);
  va_end(args);
  
  *header = curl_slist_append(*header, openai_temp_buffer);
  BLOP_ASSERTF(*header != NULL, "curl_slist_append failed -> %s", openai_temp_buffer);
}

static void   openai_push_tool(struct_openai* api, const char* name, const char* description, const cJSON* parameters) {
  BLOP_ASSERT_PTR(api);
  BLOP_ASSERT_PTR(name);
  BLOP_ASSERT_PTR(description);
  BLOP_ASSERT_PTR(parameters);

  cJSON* tool = cJSON_CreateObject();
  BLOP_ASSERT_PTR(tool);

  cJSON* fn = cJSON_AddObjectToObject(tool, "function");
  BLOP_ASSERT_PTR(fn);

  BLOP_ASSERT_PTR(cJSON_AddStringToObject(tool, "type", "function"));
  BLOP_ASSERT_PTR(cJSON_AddStringToObject(fn, "description", description));
  BLOP_ASSERT_PTR(cJSON_AddStringToObject(fn, "name", name));

  cJSON* dup_params = NULL;
  if (parameters) {
    dup_params = cJSON_Duplicate(parameters, true);
    BLOP_ASSERT_PTR(dup_params);
  } else {
    dup_params = cJSON_CreateObject();
    BLOP_ASSERT_PTR(dup_params);
  }

  cJSON_AddItemToObject(fn, "parameters", dup_params); /* ownership transferred here */
  cJSON_AddItemToArray(api->tools, tool);
}

static struct_openai_tool* openai_find_tool(struct_openai* api, const char* name) {
  BLOP_ASSERT_PTR(api);
  BLOP_ASSERT_PTR(name);
  
  for (size_t i = 0; i < OPENAI_TOOL_COUNT; ++i) {
    if (api->attached[i].in_use && strcmp(api->attached[i].name, name) == 0) {
      return &api->attached[i];
    }
  }
  return NULL;
}

/* --------------------------------------------------------------------------
 * Backends
 * -------------------------------------------------------------------------- */

static const struct_openai_backend_ops* openai_ops = NULL;

  /* --------------------------------------------------------------------------
   * Openai
   * -------------------------------------------------------------------------- */

static int openai_parse_args(cJSON* raw, cJSON** out) {
  if (!cJSON_IsString(raw)) { return 0; }
  *out = cJSON_Parse(raw->valuestring);
  return *out != NULL;
}

static void openai_push_result(struct_openai* api, const char* id, const char* name, cJSON* result) {
  cJSON* msg = cJSON_CreateObject();
  cJSON_AddStringToObject(msg, "role", "tool");
  cJSON_AddStringToObject(msg, "tool_call_id", id);

  char* printed = cJSON_PrintUnformatted(result);
  if (printed) {
    BLOP_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", printed));
    BLOP_FREE_IF(printed);
  } else {
    BLOP_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", "{}"));
  }

  fn_openai_history_insert(api, fn_openai_history_size(api), msg);
}

static void openai_init_headers(struct_openai* api) {
  openai_append_hdr(&api->headers, "Authorization: Bearer %s", api->config.api_key);
  openai_append_hdr(&api->headers, "Content-Type: application/json");
}

static int openai_supports_seed(void) { return 1; }

static const struct struct_openai_backend_ops OPENAI_BACKEND = {
  .init_request      = NULL,
  .init_headers      = openai_init_headers,
  .parse_tool_args   = openai_parse_args,
  .push_tool_result  = openai_push_result,
  .supports_seed     = openai_supports_seed
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

static void openrouter_push_result(struct_openai* api, const char* id, const char* name, cJSON* result) {
  cJSON* msg = cJSON_CreateObject();
  cJSON_AddStringToObject(msg, "role", "tool");
  cJSON_AddStringToObject(msg, "tool_call_id", id);
  cJSON_AddStringToObject(msg, "name", name);

  char* printed = cJSON_PrintUnformatted(result);
  if (printed) {
    BLOP_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", printed));
    BLOP_FREE_IF(printed);
  } else {
    BLOP_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", "{}"));
  }

  fn_openai_history_insert(api, fn_openai_history_size(api), msg);
}

static void openrouter_init_headers(struct_openai* api) {
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

static const struct struct_openai_backend_ops OPENROUTER_BACKEND = {
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

static void llama_init_headers(struct_openai* api) {
  openai_append_hdr(&api->headers, "Content-Type: application/json");
}

static int llama_supports_seed(void) { return 0; }

static const struct struct_openai_backend_ops LLAMA_BACKEND = {
  .init_request      = NULL,
  .init_headers      = llama_init_headers,
  .parse_tool_args   = llama_parse_args,
  .push_tool_result  = openai_push_result, /* same as OpenAI */
  .supports_seed     = llama_supports_seed
};

/* --------------------------------------------------------------------------
 * Implementation
 * -------------------------------------------------------------------------- */

struct_openai*  fn_openai_create(struct_openai* api, struct_openai_config config) {
  BLOP_ASSERT_PTR(config.model);

  if (!api) {
    BLOP_CALLOC(api, struct struct_openai, 1);
    api->allocated = true;
  }

  BLOP_RWLOCK_INIT(api->lock);

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

  BLOP_ASSERT_BOUNDS(api->config.max_remove_idx, api->config.max_history - 2);

  /* ------------------------------------------------------------------ */
  /* Backend selection                                                  */
  /* ------------------------------------------------------------------ */

  switch (config.backend) {
    case OPENAI_API:
      openai_ops = &OPENAI_BACKEND;
      if (!api->config.url) { api->config.url = OPENAI_DEFAULT_URL; }
      BLOP_ASSERT_PTR(api->config.api_key);
      break;

    case OPENROUTER_API:
      openai_ops = &OPENROUTER_BACKEND;
      if (!api->config.url) { api->config.url = OPENROUTER_DEFAULT_URL; }
      if (!api->config.x_title) { api->config.x_title = OPENROUTER_DEFAULT_X_TITLE; }
      BLOP_ASSERT_PTR(api->config.api_key);
      break;

    case LLAMA_CPP_API:
      openai_ops = &LLAMA_BACKEND;
      /* llama MUST provide url */
      BLOP_ASSERT_PTR(api->config.url);
      break;

    default:
      BLOP_ASSERT(0, "Unknown backend");
  }

  BLOP_ASSERT_PTR(openai_ops);

  /* ------------------------------------------------------------------ */
  /* Root JSON                                                          */
  /* ------------------------------------------------------------------ */

  api->root = cJSON_CreateObject();
  BLOP_ASSERT_PTR(api->root);

  api->tools    = cJSON_AddArrayToObject(api->root, "tools");
  api->messages = cJSON_AddArrayToObject(api->root, "messages");

  cJSON_AddStringToObject(api->root, "model", api->config.model);
  cJSON_AddNumberToObject(api->root, "temperature", api->config.temperature);
  cJSON_AddNumberToObject(api->root, "top_p", api->config.top_p);

  if (api->config.max_tokens != -1) { 
    BLOP_ASSERT_PTR(cJSON_AddNumberToObject(api->root, "max_tokens", api->config.max_tokens));
  }
  
  if (api->config.seed >= 0 && openai_ops->supports_seed()) { 
    BLOP_ASSERT_PTR(cJSON_AddNumberToObject(api->root, "seed", api->config.seed));
  }

  if (api->config.reasoning != -1 && config.backend == OPENROUTER_API) {
    cJSON* reasoning = cJSON_CreateObject();
    BLOP_ASSERT_PTR(reasoning);
    BLOP_ASSERT_PTR(cJSON_AddBoolToObject(reasoning, "enabled", !!api->config.reasoning));
    cJSON_AddItemToObject(api->root, "reasoning", reasoning);
  }

  if (api->config.presence_penalty != 0.f) {
    BLOP_ASSERT_PTR(cJSON_AddNumberToObject(api->root, "presence_penalty", api->config.presence_penalty));
  }

  if (api->config.frequency_penalty != 0.f) {
    BLOP_ASSERT_PTR(cJSON_AddNumberToObject(api->root, "frequency_penalty", api->config.frequency_penalty));
  }

  if (api->config.stop) {
    BLOP_ASSERT(
      cJSON_IsString(api->config.stop) || cJSON_IsArray(api->config.stop),
      "`stop` must be string or array"
    );
    cJSON_AddItemToObject(api->root, "stop", cJSON_Duplicate(api->config.stop, true));
  }

  if (api->config.response_format) {
    cJSON* dup_resp = cJSON_Duplicate(api->config.response_format, true);
    BLOP_ASSERT_PTR(dup_resp);
    cJSON_AddItemToObject(api->root, "response_format", dup_resp);
  }

  if (api->config.logit_bias) {
    BLOP_ASSERT(cJSON_IsObject(api->config.logit_bias), "logit_bias must be object");
    cJSON* dup_lb = cJSON_Duplicate(api->config.logit_bias, true);
    BLOP_ASSERT_PTR(dup_lb);
    cJSON_AddItemToObject(api->root, "logit_bias", dup_lb);
  }

  /* ------------------------------------------------------------------ */
  /* CURL                                                               */
  /* ------------------------------------------------------------------ */

  api->curl = curl_easy_init();
  BLOP_ASSERT(api->curl, "curl_easy_init failed");

  api->headers = NULL;
  openai_ops->init_headers(api);

  CURLcode cres = curl_easy_setopt(api->curl, CURLOPT_POST, 1L);
  BLOP_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_POST failed: %s", curl_easy_strerror(cres));

  return api;
}

void            fn_openai_destroy(struct_openai* api) {
  BLOP_ASSERT_PTR(api);

  if (api->root) { cJSON_Delete(api->root); }
  if (api->headers) { curl_slist_free_all(api->headers); }
  if (api->curl) { curl_easy_cleanup(api->curl); }

  BLOP_FREE_IF(api->response);

  BLOP_RWLOCK_DESTROY(api->lock);

  if (api->allocated) {
    BLOP_FREE(api);
  }
}

cJSON*          fn_openai_send(struct_openai* api, cJSON* message) {
  BLOP_ASSERT_PTR(api);
  BLOP_ASSERT_PTR(message);
  BLOP_ASSERT_PTR(openai_ops);
  BLOP_ASSERT_PTR(api->curl);
  BLOP_ASSERT_PTR(api->root);
  BLOP_ASSERT_PTR(api->messages);

  fn_openai_history_insert(
    api,
    fn_openai_history_size(api),
    cJSON_Duplicate(message, true)
  );

  while (true) {
    BLOP_FREE_IF(api->response);
    api->response_size = 0;

    char* payload = cJSON_PrintUnformatted(api->root);
    BLOP_ASSERT_PTR(payload);

    CURLcode cres;
    cres = curl_easy_setopt(api->curl, CURLOPT_URL, api->config.url);
    BLOP_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_URL failed: %s", curl_easy_strerror(cres));
    cres = curl_easy_setopt(api->curl, CURLOPT_HTTPHEADER, api->headers);
    BLOP_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_HTTPHEADER failed: %s", curl_easy_strerror(cres));
    cres = curl_easy_setopt(api->curl, CURLOPT_POSTFIELDS, payload);
    BLOP_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_POSTFIELDS failed: %s", curl_easy_strerror(cres));
    cres = curl_easy_setopt(api->curl, CURLOPT_WRITEFUNCTION, openai_write_callback);
    BLOP_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_WRITEFUNCTION failed: %s", curl_easy_strerror(cres));
    cres = curl_easy_setopt(api->curl, CURLOPT_WRITEDATA, api);
    BLOP_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_WRITEDATA failed: %s", curl_easy_strerror(cres));

    if (api->config.timeout_ms > 0) {
      cres = curl_easy_setopt(api->curl, CURLOPT_TIMEOUT_MS, api->config.timeout_ms);
      BLOP_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_TIMEOUT_MS failed: %s", curl_easy_strerror(cres));
    }

    if (api->config.cainfo_path) {
      cres = curl_easy_setopt(api->curl, CURLOPT_CAINFO, api->config.cainfo_path);
      BLOP_ASSERTF(cres == CURLE_OK, "curl_easy_setopt CURLOPT_CAINFO failed: %s", curl_easy_strerror(cres));
    }

    CURLcode res = curl_easy_perform(api->curl);
    BLOP_FREE(payload);

    BLOP_ASSERTF(res == CURLE_OK, "curl_easy_perform failed: %s", curl_easy_strerror(res));
    BLOP_ASSERT(api->response_size > 0, "Empty HTTP response");

    cJSON* root = cJSON_Parse(api->response);
    BLOP_ASSERT_PTR(root);

    /* -------------------------------------------------------------- */
    /* ERROR                                                          */
    /* -------------------------------------------------------------- */

    cJSON* err = cJSON_GetObjectItem(root, "error");
    if (err) {
      char* e = cJSON_Print(err);
      BLOP_ASSERTF(false, "API error:\n%s", e ? e : "<unprintable>");
    }

    /* -------------------------------------------------------------- */
    /* CHOICES                                                        */
    /* -------------------------------------------------------------- */

    cJSON* choices = cJSON_GetObjectItem(root, "choices");
    BLOP_ASSERTF(choices && cJSON_IsArray(choices), "Missing choices\n%s", api->response);

    cJSON* choice = cJSON_GetArrayItem(choices, 0);
    BLOP_ASSERT_PTR(choice);

    /* -------------------------------------------------------------- */
    /* MESSAGE (OpenAI / OpenRouter / llama.cpp compatible)           */
    /* -------------------------------------------------------------- */

    cJSON* msg = cJSON_GetObjectItem(choice, "message");

    if (!msg) {
      /* llama.cpp fallback */
      cJSON* text = cJSON_GetObjectItem(choice, "text");
      BLOP_ASSERT(text && cJSON_IsString(text), "Invalid llama.cpp response");

      msg = cJSON_CreateObject();
      BLOP_ASSERT_PTR(msg);
      BLOP_ASSERT_PTR(cJSON_AddStringToObject(msg, "role", "assistant"));
      BLOP_ASSERT_PTR(cJSON_AddStringToObject(msg, "content", text->valuestring));
    }

    BLOP_ASSERT_PTR(msg);

    /* -------------------------------------------------------------- */
    /* TOOL CALLS                                                     */
    /* -------------------------------------------------------------- */

    cJSON* tool_calls = cJSON_GetObjectItem(msg, "tool_calls");

    if (tool_calls && cJSON_IsArray(tool_calls)) {
      cJSON* dup_msg = cJSON_Duplicate(msg, true);
      BLOP_ASSERT_PTR(dup_msg);
      fn_openai_history_insert(api, fn_openai_history_size(api), dup_msg);

      cJSON* call;
      cJSON_ArrayForEach(call, tool_calls) {
        cJSON* id = cJSON_GetObjectItem(call, "id");
        cJSON* fn = cJSON_GetObjectItem(call, "function");
        BLOP_ASSERT_PTR(fn);

        cJSON* name = cJSON_GetObjectItem(fn, "name");
        cJSON* args_raw = cJSON_GetObjectItem(fn, "arguments");
        BLOP_ASSERT_PTR(name);
        BLOP_ASSERT_PTR(args_raw);

        cJSON* args = NULL;

        if (cJSON_IsString(args_raw)) {
          args = cJSON_Parse(args_raw->valuestring);
        } else if (cJSON_IsObject(args_raw)) {
          args = cJSON_Duplicate(args_raw, true);
        }

        BLOP_ASSERT(args != NULL, "Failed to parse tool arguments");

        struct_openai_tool* tool = openai_find_tool(api, name->valuestring);
        BLOP_ASSERTF(tool != NULL, "Unknown tool requested: %s", name->valuestring);
        BLOP_ASSERT_PTR(tool->fn);
        BLOP_ASSERT_PTR(tool->parameters); /* tool should have parameters object */

        cJSON* result = tool->fn(args);
        cJSON_Delete(args);
        BLOP_ASSERT_PTR(result);

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
    BLOP_ASSERT_PTR(response);
    fn_openai_history_insert(api, fn_openai_history_size(api), response);

    cJSON_Delete(root);
    return response;
  }
}

void            fn_openai_rdlock(struct_openai* api) {
  BLOP_ASSERT_PTR(api);
  BLOP_RWLOCK_RDLOCK(api->lock);
}

void            fn_openai_wrlock(struct_openai* api) {
  BLOP_ASSERT_PTR(api);
  BLOP_RWLOCK_WRLOCK(api->lock);
}

void            fn_openai_rdunlock(struct_openai* api) {
  BLOP_ASSERT_PTR(api);
  BLOP_RWLOCK_RDUNLOCK(api->lock);
}

void            fn_openai_wrunlock(struct_openai* api) {
  BLOP_ASSERT_PTR(api);
  BLOP_RWLOCK_WRUNLOCK(api->lock);
}

void            fn_openai_tool_create(struct_openai_tool* tool, const char* name, const char* description, type_openai_tool_fn fn) {
  BLOP_ASSERT_PTR(tool);
  BLOP_ASSERT_PTR(name);
  BLOP_ASSERT_PTR(description);
  BLOP_ASSERT_PTR(fn);

  tool->fn = fn;
  tool->name = name;
  tool->description = description;
  tool->parameters = cJSON_CreateObject();
  BLOP_ASSERT_PTR(tool->parameters);

  cJSON* properties = cJSON_AddObjectToObject(tool->parameters, "properties");
  BLOP_ASSERT_PTR(properties);
  BLOP_ASSERT_PTR(cJSON_AddStringToObject(tool->parameters, "type", "object"));
}

void            fn_openai_tool_add_param(struct_openai_tool* tool, const char* name, const char* type) {
  BLOP_ASSERT_PTR(tool);
  BLOP_ASSERT_PTR(name);
  BLOP_ASSERT_PTR(type);
  BLOP_ASSERT_PTR(tool->parameters);

  cJSON* properties = cJSON_GetObjectItem(tool->parameters, "properties");
  BLOP_ASSERT_PTR(properties);

  cJSON* parameter = cJSON_AddObjectToObject(properties, name);
  BLOP_ASSERT_PTR(parameter);
  BLOP_ASSERT_PTR(cJSON_AddStringToObject(parameter, "type", type));
}

void            fn_openai_tool_attach(struct_openai* api, size_t idx, struct_openai_tool tool) {
  BLOP_ASSERT_PTR(api);
  BLOP_ASSERT_BOUNDS(idx, OPENAI_TOOL_COUNT);
  BLOP_ASSERT(!api->attached[idx].in_use, "Trying to attach a tool in an already attached slot");

  BLOP_ASSERT_PTR(tool.fn);
  BLOP_ASSERT_PTR(tool.name);
  BLOP_ASSERT_PTR(tool.parameters);

  tool.in_use = true;
  if (tool.description == NULL) {
    tool.description = "No description provided";
  }

  for (size_t i = 0; i < OPENAI_TOOL_COUNT; ++i) {
    if (api->attached[i].in_use) {
      BLOP_ASSERTF(strcmp(api->attached[i].name, tool.name) != 0, "Tool name already attached: %s", tool.name);
    }
  }

  openai_push_tool(api, tool.name, tool.description, tool.parameters);
  api->attached[idx] = tool;
}

void            fn_openai_tool_deattach(struct_openai* api, size_t idx) {
  BLOP_ASSERT_PTR(api);
  BLOP_ASSERT_BOUNDS(idx, OPENAI_TOOL_COUNT);
  BLOP_ASSERT_FORCED(api->attached[idx].in_use, "Trying to deattach a tool in a free slot");

  cJSON_DeleteItemFromObject(api->root, "tools");
  api->tools = cJSON_AddArrayToObject(api->root, "tools");
  BLOP_ASSERT_PTR(api->tools);
  memset(&api->attached[idx], 0, sizeof(struct_openai_tool));

  for (size_t i = 0; i < OPENAI_TOOL_COUNT; i++) {
    if (api->attached[i].in_use) {
      openai_push_tool(api, api->attached[i].name, api->attached[i].description, api->attached[i].parameters);
    }
  }
}

void            fn_openai_history_set(struct_openai* api, size_t idx, cJSON* message) {
  BLOP_ASSERT_PTR(api);
  BLOP_ASSERT_PTR(message);
  BLOP_ASSERT_PTR(api->messages);
  BLOP_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages));
  cJSON_ReplaceItemInArray(api->messages, idx, message);
}

cJSON*          fn_openai_history_get(struct_openai* api, size_t idx) {
  BLOP_ASSERT_PTR(api);
  BLOP_ASSERT_PTR(api->messages);
  BLOP_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages));
  return cJSON_GetArrayItem(api->messages, idx);
}

void            fn_openai_history_erase(struct_openai* api, size_t idx) {
  BLOP_ASSERT_PTR(api);
  BLOP_ASSERT_PTR(api->messages);
  BLOP_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages));
  cJSON_DeleteItemFromArray(api->messages, idx);
}

void            fn_openai_history_insert(struct_openai* api, size_t idx, cJSON* message) {
  BLOP_ASSERT_PTR(api);
  BLOP_ASSERT_PTR(message);
  BLOP_ASSERT_PTR(api->messages);
  BLOP_ASSERT_BOUNDS(idx, cJSON_GetArraySize(api->messages) + 1);

  cJSON_InsertItemInArray(api->messages, idx, message);

  while (cJSON_GetArraySize(api->messages) > api->config.max_history) {
    cJSON_DeleteItemFromArray(api->messages, api->config.max_remove_idx);
  }
}

int             fn_openai_history_size(struct_openai* api) {
  BLOP_ASSERT_PTR(api);
  BLOP_ASSERT_PTR(api->messages);
  return cJSON_GetArraySize(api->messages);
}

#endif /* OPENAI_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* __MYSTD_OPENAI_H__ */