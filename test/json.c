#include <mystd/stdlib.c>
#include <mystd/string.c>
#include <mystd/json.c>

int main() {
    MyJsonRoot* root = MyJsonRoot_Create(NULL, MY_JSON_OBJECT);
    MyJson* body = root->body;
    MyJson_KInsertNull(body, "none");
    MyJson_KInsertBool(body, "bool1", true);
    MyJson_KInsertBool(body, "bool2", false);
    MyJson_KInsertInteger(body, "integer1", 256);
    MyJson_KInsertDecimal(body, "decimal2", 256.789);
    MyJson_KInsertDecimal(body, "decimal3", 3.14159265);
    
    MyJson* text = MyJson_KInsertObject(body, "text");
    MyJson_KInsertString(text, "hola", "Hola mundo!");
    MyJson_KInsertString(text, "new", "Hola mundo!\n");
    MyJson_KInsertString(text, "todo", "que tal todo!");

    char* pretty = MyJsonRoot_Print(root, true);
    char* prettynt = MyJsonRoot_Print(root, false);
    MyJsonRoot_Destroy(root);

    // MyFilePrint(MyStdout(), "Pretty:\n");
    // MyFilePrint(MyStdout(), pretty);
    // MyFilePrint(MyStdout(), "Prettynt:\n");
    // MyFilePrint(MyStdout(), prettynt);
    
    MyJsonRoot* prettyJson = MyJsonRoot_Parse(NULL, pretty);
    MyJsonRoot* prettyntJson = MyJsonRoot_Parse(NULL, prettynt);

    MyFilePrint(MyStdout(), MyJsonRoot_Print(prettyJson, true));
    MyFilePrint(MyStdout(), MyJsonRoot_Print(prettyntJson, false));
    return 0;
}