#include "mystd/stddef.h"
#include <mystd/loader.h>

MY_RWLOCK_DEFINES(MyLoaderFile, file, MyLoader)

MyLoaderFile* MyLoader_Create(MyLoaderFile* file, const char* filepath) {
    size_t len = strlen(filepath);
    MY_ASSERT(len < MY_LOADER_FILEPATH_SIZE, MySprintf("Filepath lenght (%zu) exceeds MY_LOADER_FILEPATH_SIZE (%zu)", len, MY_LOADER_FILEPATH_SIZE));
    
    MY_STRUCT_CREATE_RULE(file, MyLoaderFile);

    file->open = false;
    file->mode = MY_LOADER_NULL;
    file->cursor = 0;
    memcpy(file->filepath, filepath, len + 1);
    MyString_Create(&file->data);
    return file;
}
void MyLoader_Destroy(MyLoaderFile* file) {
    MY_ASSERT_PTR(file);

    MyLoader_Close(file);
    MyString_Clear(&file->data);
    MyString_Destroy(&file->data);
    MY_STRUCT_DESTROY_RULE(file);
}

void MyLoader_Open(MyLoaderFile* file, MyLoaderMode mode) {
    MY_ASSERT_PTR(file);
    MY_ASSERT(!file->open, "Attempting to reopen a MyLoaderFile");
    MY_ASSERT(mode != MY_LOADER_NULL, "Open mode can not be MY_LOADER_NULL");

    file->open = true;
    file->mode = mode;
    file->cursor = 0;
    MyString_Clear(&file->data);

    if (mode == MY_LOADER_WRITE) {
        return;
    }

    MY_ASSERT(MyFileExists(file->filepath), MySprintf("{%s} does not exists", file->filepath));
    size_t size = 0;
    uint8_t* raw = MyFileDump(file->filepath, &size);
    MyString_Resize(&file->data, size);
    MyString_Memcpy(&file->data, 0, raw, size);
    MY_FREE(raw);
}
void MyLoader_Close(MyLoaderFile* file) {
    MY_ASSERT_PTR(file);
    
    if (file->mode != MY_LOADER_WRITE || !file->open) {
        return;
    }

    MyFile* f = MyFileOpen(file->filepath, MY_FILE_FLAG_WRITE | MY_FILE_FLAG_NEW);
    MyFileWrite(f, MyString_Cstr(&file->data), MyString_Size(&file->data));
    MyFileClose(f);
    file->open = false;
    file->mode = MY_LOADER_NULL;
}

size_t MyLoader_GetCursor(MyLoaderFile* file) {
    MY_ASSERT_PTR(file);
    return file->cursor;
}
void MyLoader_SetCursor(MyLoaderFile* file, size_t pos) {
    MY_ASSERT_PTR(file);
    file->cursor = pos;
}
void MyLoader_CursorStart(MyLoaderFile* file) {
    MY_ASSERT_PTR(file);
    file->cursor = 0;
}
void MyLoader_CursorEnd(MyLoaderFile* file) {
    MY_ASSERT_PTR(file);
    file->cursor = file->data.size;
}

void MyLoader_Write(MyLoaderFile* file, const void* data, size_t size) {
    MY_ASSERT_PTR(file);
    MY_ASSERT(file->open, "MyLoaderFile was not open");
    MY_ASSERT(file->mode == MY_LOADER_WRITE, "MyLoaderFile was not open in write mode");

    if (file->cursor + size > file->data.size) {
        MyString_Resize(&file->data, file->cursor + size);
    }

    MyString_Memcpy(&file->data, file->cursor, data, size);
    file->cursor += size;
}
void MyLoader_Read(MyLoaderFile* file, void* data, size_t size) {
    MY_ASSERT_PTR(file);
    MY_ASSERT(file->open, "MyLoaderFile was not open");
    MY_ASSERT(file->cursor + size <= file->data.size, "MyLoader read overflow");

    memcpy(data, &file->data.data[file->cursor], size);
    file->cursor += size;
}

void MyLoader_SaveU8(MyLoaderFile* file, uint8_t data) {
    MyLoader_Write(file, &data, sizeof(data));
}
void MyLoader_SaveU16(MyLoaderFile* file, uint16_t data) {
    MyLoader_Write(file, &data, sizeof(data));
}
void MyLoader_SaveU32(MyLoaderFile* file, uint32_t data) {
    MyLoader_Write(file, &data, sizeof(data));
}
void MyLoader_SaveU64(MyLoaderFile* file, uint64_t data) {
    MyLoader_Write(file, &data, sizeof(data));
}
void MyLoader_SaveF32(MyLoaderFile* file, float data) {
    MyLoader_Write(file, &data, sizeof(data));
}
void MyLoader_SaveF64(MyLoaderFile* file, double data) {
    MyLoader_Write(file, &data, sizeof(data));
}
void MyLoader_SaveSTR(MyLoaderFile* file, const char* data, size_t size) {
    MyLoader_Write(file, data, size);
}

uint8_t MyLoader_LoadU8(MyLoaderFile* file) {
    uint8_t data = 0;
    MyLoader_Read(file, &data, sizeof(data));
    return data;
}
uint16_t MyLoader_LoadU16(MyLoaderFile* file) {
    uint16_t data = 0;
    MyLoader_Read(file, &data, sizeof(data));
    return data;
}
uint32_t MyLoader_LoadU32(MyLoaderFile* file) {
    uint32_t data = 0;
    MyLoader_Read(file, &data, sizeof(data));
    return data;
}
uint64_t MyLoader_LoadU64(MyLoaderFile* file) {
    uint64_t data = 0;
    MyLoader_Read(file, &data, sizeof(data));
    return data;
}
float MyLoader_LoadF32(MyLoaderFile* file) {
    float data = 0;
    MyLoader_Read(file, &data, sizeof(data));
    return data;
}
double MyLoader_LoadF64(MyLoaderFile* file) {
    double data = 0;
    MyLoader_Read(file, &data, sizeof(data));
    return data;
}
void MyLoader_LoadSTR(MyLoaderFile* file, char* buffer, size_t size) {
    MyLoader_Read(file, buffer, size);
}