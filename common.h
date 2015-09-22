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
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char sbyte;
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

#define var auto

#define va_begin(from)\
    va_list args;\
    va_start(args, from);

#define EXPAND(x) x

#define arrcount(x) (sizeof(x)/sizeof(x[0])

#define streql(x, y) (strcmp((x), (y)) == 0)

#define COMPARE(x, y) (((x) > (y)) - ((x) < (y)))

#define FUNC(T, Name, Params, code)\
    struct {\
        typedef T(*Name##fun_t) Params; \
        operator Name##fun_t() { return _fun_; }\
        static T _fun_ Params code\
        T operator () Params code\
    } Name

#define SETMEM(ptr, count, Value) do { for (int i = 0; i < count; i++) { ptr[i] = Value; } } while (0)

#define ZEROMEM(ptr, count) SETMEM(ptr, count, 0)

#define SWAP(T, x, y) do { T temp = x; x = y; y = temp; } while (0)

#define FORi(Count) for(u32 i = 0; i < Count; i++)
#define FORj(Count) for(u32 j = 0; j < Count; j++)
#define FORk(Count) for(u32 k = 0; k < Count; k++)
#define FOR(Count) FORi(Count)

#define FOREACH0(item, array, start, Count)\
for (u32 i = (start), keep = 1; \
    keep && i < (Count); \
    keep = !keep, i++)\
for (item = (array)[i]; keep; keep = !keep)

#define FOREACH1(item, array, Count)\
    FOREACH0(item, array, 0, Count)

#define FOREACH2(item, array)\
for (u32 i = 0, keep = 1; \
    keep && i < ArrayCount(array); \
    keep = !keep, i++)\
for (item = (array)[i]; keep; keep = !keep)

#define FOREACH3(array) FOREACH2(var it, array)

#define GET_FOREACH(_0, _1, _2, _3, NAME, ...) NAME
#define FOREACH(...) EXPAND(GET_FOREACH(__VA_ARGS__, FOREACH0, FOREACH1, FOREACH2, FOREACH3)(__VA_ARGS__))

#define MALLOCX(T, n) ((T*)malloc((n) * sizeof(T)))
#define MALLOC(T) ((T*)malloc(sizeof(T)))
#define CALLOC(T, n) ((T*)calloc((n), sizeof(T)))
#define SALLOC(n) CALLOC(char, n)
#define RALLOC(x, T, n) (T*)realloc((x), (n) * sizeof(T))

#define DEFINE_LINEAR_SEARCH_POSTFIX(T, postfix)\
    int LINEAR_SEARCH_##postfix(T* array, u32 count, T Value, s32(*compare)(T, T))\
    {\
        for (u32 i = 0; i < count; i++)\
            if (compare((array)[i], Value) == 0)\
                return i; \
        return -1; \
    }

#define DEFINE_LINEAR_SEARCH(T)\
    DEFINE_LINEAR_SEARCH_POSTFIX(T, T)

#define LINEAR_SEARCH(T, array, count, Value, compare)\
    LINEAR_SEARCH_##T(array, count, Value, compare)

#define STRING_SEARCH(array, count, Value)\
    LINEAR_SEARCH(string, array, count, Value, cmp_string)

inline int cmp_u32(u32 x, u32 y) { return COMPARE(x, y); }
inline int cmp_string(string x, string y) { return strcmp(x, y); }

#define USING(obj, data)\
    for (u8 done = obj.begin_using(data); !done; done = obj.end_using(data))

DEFINE_LINEAR_SEARCH(u32)
DEFINE_LINEAR_SEARCH(string)
DEFINE_LINEAR_SEARCH_POSTFIX(void*, voidptr)

//TODO not portable
#define FILE_NAME (strrchr(__FILE__, '\\'))

#ifdef NDEBUG
#define Assert(...)
#else

static char _Error[1024];

#define Abort() *(int*)0 = 0

#define Assert0(Expression) (Expression ? 1 : \
        Abort(),\
        0)

#define Assert1(Expression, Message) (Expression ? 1 : \
        fprintf(stderr, "(%s: `%s` at %d) Error: %s\n", FILE_NAME, __FUNCTION__, __LINE__, Message), \
        Abort(), \
        0)

#define AssertN(Expression, Message, ...) (Expression ? 1 : \
        sprintf(_Error, "(%s: `%s` at %d) Error: %s\n", FILE_NAME, __FUNCTION__, __LINE__, Message), \
        fprintf(stderr, _Error, __VA_ARGS__), \
        Abort(), \
        0)

#define GET_ASSERT(_0, _1, _2, _3, _4, _5, _6, NAME, ...) NAME
#define Assert(...) EXPAND(GET_ASSERT(__VA_ARGS__, AssertN, AssertN, AssertN, AssertN, AssertN, Assert1, Assert0)(__VA_ARGS__))
#endif

size_t GetFileSize(FILE* file)
{
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    rewind(file);
    return filesize;
}

string ReadTextFile(string filename)
{
    if (!filename) return null;
    FILE* file = fopen(filename, "rb");
    if (!file) return null;
    
    size_t filesize = GetFileSize(file);
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

#define ArrayCount(A) A##Count
#define ArrayLast(A) A[A##Count - 1]
#define ArrayZero(A, N) FOR(N) A[i] = 0
#define ArrayAdd(A, Value) A[A##Count++] = Value

#define StaticArray(T, Name, N) \
    T Name[N]; \
    u32 Name##Count

#define DynamicArray(Type, Name) \
    Type* Name; \
    u32   Name##Count

inline bool strempty(string s)
{
    return strlen(s) == 0;
}

inline s32 stridx(string str, char c)
{
    string res = strchr(str, c);
    return res ? res - str : -1;
}

inline void strtrim(string& s)
{
    while(isspace(*s))
        s++;

    u32 i = strlen(s) - 1;
    while(*s && isspace(s[i]) && i >= 0)
    {
        s[i] = '\0';
        i--;
    }
}

inline char strlast(string s)
{
    return s[strlen(s) - 1];
}

u32 strsplit(string input, char delim, string* &result)
{
    const u32 MAX = 256;
    u32 locations[MAX];
    u32 finds = 0;
    u32 total_length = 0;
    string s = input;

    while(*s)
    {
        if (*s == delim)
            locations[finds++] = s - input;
        s++;
    }

    result = CALLOC(string, finds+1);

    if (finds == 0)
    {
        strtrim(input);
        result[0] = input;
        return 1;
    }

    total_length = s - input;

    s = input;
    u32 next = 0;
    while(*s)
    {
        u32 split_length;
        if (next == 0)
            split_length = locations[0];
        else if (next == finds)
            split_length = total_length - locations[next-1] - 1;
        else
            split_length = locations[next] - locations[next-1] - 1;

        string split =  SALLOC(split_length + 1);
        memcpy(split, s, split_length);
        strtrim(split);
        if (!strempty(split))
        {
            result[next] = split;
            result[next][split_length] = '\0';
            s += split_length + (next < finds);
            next++;
        }
        else
        {
            s += split_length + (next < finds);
            finds--;
        }
    }

    return finds + 1;
}

#endif // COMMON_H
