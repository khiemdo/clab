#include <stdio.h>

typedef char* string;

#define StaticArray(T, Name, N) \
    T Name[N]; \
    unsigned int Name##Count = 0

struct function_declaration
{
    string Type;
    string Name;
    StaticArray(string, ParamNames, 32);
    StaticArray(string, ParamTypes, 32);
};

void main()
{
    function_declaration dec = {};
    printf("", dec.Name);
    getchar();
}
