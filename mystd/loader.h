#ifndef __MYSTD_LOADER_H__
#define __MYSTD_LOADER_H__

#include "stddef.h"
#include <mystd/stdio.h>
#include <mystd/string.h>

#ifndef MY_LOADER_FILEPATH_SIZE 
    #define MY_LOADER_FILEPATH_SIZE 512
#endif /* MY_LOADER_FILEPATH_SIZE */

typedef enum {
    MY_LOADER_NULL,
    MY_LOADER_WRITE,
    MY_LOADER_READ
} MyLoaderMode;

typedef struct MyLoaderFile {
    MyStructHeader  header;

    char            filepath[MY_LOADER_FILEPATH_SIZE];
    MyString        data;
    size_t          cursor;
    MyLoaderMode    mode;
    bool8           open;
} MyLoaderFile;

MY_RWLOCK_DECLARES(MyLoaderFile, file, MyLoader)

MyLoaderFile*   MyLoader_Create         (MyLoaderFile* file, const char* filepath);
void            MyLoader_Destroy        (MyLoaderFile* file);

void            MyLoader_Open           (MyLoaderFile* file, MyLoaderMode mode);
void            MyLoader_Close          (MyLoaderFile* file);

size_t          MyLoader_GetCursor      (MyLoaderFile* file);
void            MyLoader_SetCursor      (MyLoaderFile* file, size_t pos);
void            MyLoader_CursorStart    (MyLoaderFile* file);
void            MyLoader_CursorEnd      (MyLoaderFile* file);

void            MyLoader_Write          (MyLoaderFile* file, const void* data, size_t size);
void            MyLoader_Read           (MyLoaderFile* file, void* data, size_t size);

void            MyLoader_SaveU8         (MyLoaderFile* file, uint8_t data);
void            MyLoader_SaveU16        (MyLoaderFile* file, uint16_t data);
void            MyLoader_SaveU32        (MyLoaderFile* file, uint32_t data);
void            MyLoader_SaveU64        (MyLoaderFile* file, uint64_t data);
void            MyLoader_SaveF32        (MyLoaderFile* file, float data);
void            MyLoader_SaveF64        (MyLoaderFile* file, double data);
void            MyLoader_SaveSTR        (MyLoaderFile* file, const char* data, size_t size);

uint8_t         MyLoader_LoadU8         (MyLoaderFile* file);
uint16_t        MyLoader_LoadU16        (MyLoaderFile* file);
uint32_t        MyLoader_LoadU32        (MyLoaderFile* file);
uint64_t        MyLoader_LoadU64        (MyLoaderFile* file);
float           MyLoader_LoadF32        (MyLoaderFile* file);
double          MyLoader_LoadF64        (MyLoaderFile* file);
void            MyLoader_LoadSTR        (MyLoaderFile* file, char* buffer, size_t size);

#define MyLoader_Save(file, data)           \
    _Generic((data),                        \
        bool:           MyLoader_SaveU8,    \
        uint8_t:        MyLoader_SaveU8,    \
        int8_t:         MyLoader_SaveU8,    \
        uint16_t:       MyLoader_SaveU16,   \
        int16_t:        MyLoader_SaveU16,   \
        uint32_t:       MyLoader_SaveU32,   \
        int32_t:        MyLoader_SaveU32,   \
        uint64_t:       MyLoader_SaveU64,   \
        int64_t:        MyLoader_SaveU64,   \
        float:          MyLoader_SaveF32,   \
        double:         MyLoader_SaveF64    \
    )(file, data)

#define MyLoader_Load(file, pData)          \
    ((*pData) = _Generic((pData),           \
        bool*:          MyLoader_LoadU8,    \
        uint8_t*:       MyLoader_LoadU8,    \
        int8_t*:        MyLoader_LoadU8,    \
        uint16_t*:      MyLoader_LoadU16,   \
        int16_t*:       MyLoader_LoadU16,   \
        uint32_t*:      MyLoader_LoadU32,   \
        int32_t*:       MyLoader_LoadU32,   \
        uint64_t*:      MyLoader_LoadU64,   \
        int64_t*:       MyLoader_LoadU64,   \
        float*:         MyLoader_LoadF32,   \
        double*:        MyLoader_LoadF64    \
    )(file))

#define MyLoader_Saveu8(file, data)  MyLoader_SaveU8(file, (uint8_t)data) 
#define MyLoader_Saveu16(file, data) MyLoader_SaveU16(file, (uint16_t)data)
#define MyLoader_Saveu32(file, data) MyLoader_SaveU32(file, (uint32_t)data)
#define MyLoader_Saveu64(file, data) MyLoader_SaveU64(file, (uint64_t)data)
#define MyLoader_Savef32(file, data) MyLoader_SaveF32(file, (float)data)
#define MyLoader_Savef64(file, data) MyLoader_SaveF64(file, (double)data)

#define MyLoader_Loadu8(file, data)  MyLoader_LoadU8(file, (uint8_t)data) 
#define MyLoader_Loadu16(file, data) MyLoader_LoadU16(file, (uint16_t)data)
#define MyLoader_Loadu32(file, data) MyLoader_LoadU32(file, (uint32_t)data)
#define MyLoader_Loadu64(file, data) MyLoader_LoadU64(file, (uint64_t)data)
#define MyLoader_Loadf32(file, data) MyLoader_LoadF32(file, (float)data)
#define MyLoader_Loadf64(file, data) MyLoader_LoadF64(file, (double)data)

#define MyLoader_Savebool(file, data)   MyLoader_SaveU8(file, (uint8_t)data) 
#define MyLoader_Savechar(file, data)   MyLoader_SaveU8(file, (uint8_t)data) 
#define MyLoader_Saveshort(file, data)  MyLoader_SaveU16(file, (uint16_t)data)
#define MyLoader_Saveint(file, data)    MyLoader_SaveU32(file, (uint32_t)data)
#define MyLoader_Savefloat(file, data)  MyLoader_SaveF32(file, (float)data)
#define MyLoader_Savedouble(file, data) MyLoader_SaveF64(file, (double)data)

#define MyLoader_Loadbool(file, data)   MyLoader_LoadU8(file, (uint8_t)data) 
#define MyLoader_Loadchar(file, data)   MyLoader_LoadU8(file, (uint8_t)data) 
#define MyLoader_Loadshort(file, data)  MyLoader_LoadU16(file, (uint16_t)data)
#define MyLoader_Loadint(file, data)    MyLoader_LoadU32(file, (uint32_t)data)
#define MyLoader_Loadfloat(file, data)  MyLoader_LoadF32(file, (float)data)
#define MyLoader_Loaddouble(file, data) MyLoader_LoadF64(file, (double)data)

#define MyLoader_Loaduint8_t(file, data)  MyLoader_LoadU8(file, (uint8_t)data) 
#define MyLoader_Loaduint16_t(file, data) MyLoader_LoadU16(file, (uint16_t)data)
#define MyLoader_Loaduint32_t(file, data) MyLoader_LoadU32(file, (uint32_t)data)
#define MyLoader_Loaduint64_t(file, data) MyLoader_LoadU64(file, (uint64_t)data)

#define MyLoader_Saveuint8_t(file, data)  MyLoader_SaveU8(file, (uint8_t)data) 
#define MyLoader_Saveuint16_t(file, data) MyLoader_SaveU16(file, (uint16_t)data)
#define MyLoader_Saveuint32_t(file, data) MyLoader_SaveU32(file, (uint32_t)data)
#define MyLoader_Saveuint64_t(file, data) MyLoader_SaveU64(file, (uint64_t)data)

#define MyLoader_Loadint8_t(file, data)  MyLoader_LoadU8(file, (uint8_t)data) 
#define MyLoader_Loadint16_t(file, data) MyLoader_LoadU16(file, (uint16_t)data)
#define MyLoader_Loadint32_t(file, data) MyLoader_LoadU32(file, (uint32_t)data)
#define MyLoader_Loadint64_t(file, data) MyLoader_LoadU64(file, (uint64_t)data)

#define MyLoader_Saveint8_t(file, data)  MyLoader_SaveU8(file, (uint8_t)data) 
#define MyLoader_Saveint16_t(file, data) MyLoader_SaveU16(file, (uint16_t)data)
#define MyLoader_Saveint32_t(file, data) MyLoader_SaveU32(file, (uint32_t)data)
#define MyLoader_Saveint64_t(file, data) MyLoader_SaveU64(file, (uint64_t)data)

#endif /* __MYSTD_LOADER_H__ */