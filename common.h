#ifndef COMMON_H
#define COMMON_H
#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_WARNINGS

#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <stdarg.h>

typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
typedef float r32;
typedef double r64;
typedef char* string;

#define null NULL
#define internal static
#define global static

#define va_begin(from)\
    va_list args; \
    va_start(args, from);

#define arrcount(x) (sizeof(x)/sizeof(x[0])

#define streql(x, y) (strcmp((x), (y)) == 0)

#define COMPARE(x, y) (((x) > (y)) - ((x) < (y)))

#define FUNC(T, name, params, code)\
    struct {\
        typedef T(*name##fun_t) params; \
        operator name##fun_t() { return _fun_; }\
        static T _fun_ params code\
        T operator () params code\
    } name

#define SETMEM(ptr, count, value) do { for (int i = 0; i < count; i++) { ptr[i] = value; } } while (0)

#define ZEROMEM(ptr, count) SETMEM(ptr, count, 0)

#define SWAP(T, x, y) do { T temp = x; x = y; y = temp; } while (0)

#define FORi(count) for(u32 i = 0; i < count; i++)
#define FORj(count) for(u32 j = 0; j < count; j++)
#define FORk(count) for(u32 k = 0; k < count; k++)
#define FOR(count) FORi(count)

#define FOREACH(item, array,  count)\
    FOREACHS(item, array, 0, count)

#define FOREACHS(item, array, start, count)\
for (u32 i = (start), keep = 1; \
    keep && i < (count); \
    keep = !keep, i++)\
for (item = (array)[i]; keep; keep = !keep)

#define FOREACHA(item, array)\
for (u32 i = 0, keep = 1; \
    keep && i < (array)->count; \
    keep = !keep, i++)\
for (item = (array)[i]; keep; keep = !keep)

#define MALLOCX(T, n) ((T*)malloc((n) * sizeof(T)))
#define MALLOC(T) ((T*)malloc(sizeof(T)))
#define CALLOC(T, n) ((T*)calloc((n), sizeof(T)))
#define SALLOC(n) CALLOC(char, n)
#define RALLOC(x, T, n) (T*)realloc((x), (n) * sizeof(T))

#define DEFINE_LINEAR_SEARCH_POSTFIX(T, postfix)\
    int LINEAR_SEARCH_##postfix(T* array, u32 count, T value, s32(*compare)(T, T))\
    {\
        for (u32 i = 0; i < count; i++)\
            if (compare((array)[i], value) == 0)\
                return i; \
        return -1; \
    }

#define DEFINE_LINEAR_SEARCH(T)\
    DEFINE_LINEAR_SEARCH_POSTFIX(T, T)

#define LINEAR_SEARCH(T, array, count, value, compare)\
    LINEAR_SEARCH_##T(array, count, value, compare)

#define STRING_SEARCH(array, count, value)\
    LINEAR_SEARCH(string, array, count, value, cmp_string)

inline int cmp_u32(u32 x, u32 y) { return COMPARE(x, y); }
inline int cmp_string(string x, string y) { return strcmp(x, y); }

#define USING(obj, data)\
    for (u8 done = obj.begin_using(data); !done; done = obj.end_using(data))

DEFINE_LINEAR_SEARCH(u32)
DEFINE_LINEAR_SEARCH(string)
DEFINE_LINEAR_SEARCH_POSTFIX(void*, voidptr)

//TODO not portable
#define FILE_EXT(path) (strrchr(path, '\\') + 1)

#ifndef NDEBUG
int assert(int exp, string msg = null, ...)
{
    if (!exp)
    {
        va_begin(msg);
        string error = SALLOC(1024);
        sprintf(error, "(%s: `%s` at %d) Error: %s", FILE_EXT(__FILE__), __FUNCTION__, __LINE__, msg);
        vfprintf(stderr, error, args);
        va_end(args);
        abort();
    }
    return 1;
}
//int assert(void* ptr, string msg = null, ...)
//{
//    va_begin(msg);
//    int ret = assert(ptr != null, msg, args);
//    va_end(args);
//    return ret;
//}
#else
#define assert(...)
#endif

#endif // COMMON_H

size_t fsize(FILE* file)
{
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    rewind(file);
    return filesize;
}

string read_text_file(string filename)
{
    if (!filename) return null;
    FILE* file = fopen(filename, "rb");
    if (!file) return null;
    
    size_t filesize = fsize(file);
    string content = MALLOCX(char, filesize);
    size_t nread = fread(content, sizeof(char), filesize, file);
    fclose(file);
    if (filesize != nread)
    {
        free(content);
        return null;
    }
    
    content[filesize] = '\0';
    return content;
}

s32 stridx(string str, char c)
{
    string res = strchr(str, c);
    return res ? res - str : -1;
}
