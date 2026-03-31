#include <mystd/stdlib.c>
#include <mystd/string.c>
#include <mystd/json.c>

int main() {
    char* json = MyFileDump("C:/Dev/mystd/test/github_example.json", NULL);
    MyJsonRoot root = {0};
    MyJsonRoot_Parse(&root, json);
    MY_FREE(json);
    MyJson* body = root.body;

    char* name = MyJson_String(MyJson_KGetPath(body, "[0].commit.committer.name"));
    MyPrintf("[0].commit.commiter.name = %s\n\n", name);

    char* url = MyJson_String(MyJson_KGetPath(body, "[0].parents[0].url"));
    MyPrintf("[0].parents[0].url = %s\n\n", url);

    return 0;
}