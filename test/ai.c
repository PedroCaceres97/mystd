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

const char* CharacterSend(Character* character, const char* msg) {
    MyAIAPIChat_Push(&character->chat, AI_USER(msg));
    return MyAIAPI_Submit(&api, &character->chat);
}
void CharacterChat(Character* character) {
    char input[2048] = {0};
    MyPrintf("%s's Input > ", character->name);
    MyFileRead(MyStdin(), input, sizeof(input));
    MyPrintf("\n%s Says:\n", character->name);
    MyFilePrint(MyStdout(), CharacterSend(character, input));
    MyFilePrint(MyStdout(), "\n\n");
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    MyAIAPI_Create(&api, apiConfig);
    
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    MyAIAPIChatConfig config = MyAIAPI_DefaultChatConfig(2);
    config.maxTokens = 100;

    Character Sebastian = {0};
    Sebastian.name = "Sebastian";
    MyAIAPIChat_Create(&Sebastian.chat, config);
    MyAIAPIChat_Push(&Sebastian.chat, AI_SYSTEM("Your name is Sebastian you must make Ernesto laugh at any cost. What joke do you tell him?"));

    Character Ernesto = {0};
    Ernesto.name = "Ernesto";
    MyAIAPIChat_Create(&Ernesto.chat, config);
    MyAIAPIChat_Push(&Ernesto.chat, AI_SYSTEM("Your name is Ernesto you must make Sebastian laugh at any cost. He says to you:"));

    while (true) {
        MyFilePrint(MyStdout(), "Sebastian: \n");
        const char* sebastian = MyAIAPI_Submit(&api, &Sebastian.chat);
        MyAIAPIChat_Push(&Ernesto.chat, AI_USER(sebastian));
        MyFilePrint(MyStdout(), sebastian);
        MyFilePrint(MyStdout(), "\n\n");
        
        MyFilePrint(MyStdout(), "Ernesto: \n");
        const char* ernesto = MyAIAPI_Submit(&api, &Ernesto.chat);
        MyAIAPIChat_Push(&Sebastian.chat, AI_USER(ernesto));
        MyFilePrint(MyStdout(), ernesto);
        MyFilePrint(MyStdout(), "\n\n");
    }
    return 0;
}