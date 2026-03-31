#include "mystd/aiapi.h"
#include <mystd/stdlib.c>
#include <mystd/string.c>
#include <mystd/json.c>
#include <mystd/aiapi.c>

const MyAIAPIConfig apiConfig = {
    .backend        = MY_AIAPI_LLAMA_CPP,
    .url            = "http://127.0.0.1:8080/v1/chat/completions",
    .model          = "local-model",
    .apiKey         = NULL,
    .xtitle         = NULL,
    .httpReferer    = NULL,
    .cainfoPath     = NULL,
    .timeoutms      = 0
};
MyAIAPI api = {0};

typedef struct {
    const char* name;
    MyAIAPIChat chat;
} Character;

void CharacterInit(Character* character, const char* mood) {
    MyAIAPIChatConfig config = MyAIAPI_DefaultChatConfig(1);
    MyAIAPIChat_Create(&character->chat, config);
    MyAIAPIChat_Push(&character->chat, AI_SYSTEM(MySprintf("Your name is %s and your current mood is %s", character->name, mood)));
}
const char* CharachterSend(Character* character, const char* msg) {
    MyAIAPIChat_Push(&character->chat, AI_USER(msg));
    return MyAIAPI_Submit(&api, &character->chat);
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    MyAIAPI_Create(&api, apiConfig);
    
    Character Ernesto = {0};
    Ernesto.name = "Ernesto";
}