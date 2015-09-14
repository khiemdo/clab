struct LIST_string
{
    u32     count; 
    u32     capacity; 
    string*     buffer; 
    void    (*clrfun)(string* element); 
    s32     (*cmpfun)(string x, string y); 
    
    LIST_string(u32 capacity, 
            void(*clrfun)(string*) = null, 
            s32(*cmpfun)(string, string) = null)
    {
        this->count = 0; 
        this->capacity = capacity; 
        this->buffer = CALLOC(string, capacity); 
        this->clrfun = clrfun; 
        this->cmpfun = cmpfun; 
    }
    
    bool empty()
    {
        return count == 0; 
    }
    
    string pop()
    {
        string ret = last(); 
        removeat(count - 1); 
        return ret; 
    }
    
    string dequeue()
    {
        string ret = first(); 
        removeat(0); 
        return ret; 
    }
    
    string first()
    {
        return buffer[0]; 
    }
    
    string last()
    {
        return buffer[count - 1]; 
    }
    
    void destroy()
    {
        free(buffer); 
    }
    
    void add(string value)
    {
        if (count >= capacity)
        {
            capacity <<= 1; 
            buffer = RALLOC(buffer, string, capacity); 
        }
        buffer[count++] = value; 
    }
    
    void removeat(u32 index)
    {
        for (u32 i = index, cnt = count - index - 1; cnt > 0; i++, cnt--)
        buffer[i] = buffer[i + 1]; 
        if (clrfun) clrfun(&buffer[count - 1]); 
        count--; 
    }
    
    s32 find(string value)
    {
        FOR(count)
            if (cmpfun(buffer[i], value))
                return i;
        return -1;
    }
    
    void remove(string value)
    {
        s32 index = find(value); 
        if (index == -1)
        return; 
        removeat(index); 
    }
    
    void clear()
    {
        if (clrfun)
        FOR(count)
        clrfun(&buffer[i]); 
        count = 0; 
    }
    
    string& at(u32 index)
    {
        return buffer[index]; 
    }
    
    string& operator [](u32 index)
    {
        return buffer[index]; 
    }
};

struct LIST_int
{
    u32     count; 
    u32     capacity; 
    int*     buffer; 
    void    (*clrfun)(int* element); 
    s32     (*cmpfun)(int x, int y); 

    LIST_int()
    {
        capacity = 4;
        buffer = CALLOC(int, capacity);
    }
    
    LIST_int(u32 capacity, 
            void(*clrfun)(int*) = null, 
            s32(*cmpfun)(int, int) = null)
    {
        this->count = 0; 
        this->capacity = capacity; 
        this->buffer = CALLOC(int, capacity); 
        this->clrfun = clrfun; 
        this->cmpfun = cmpfun; 
    }
    
    bool empty()
    {
        return count == 0; 
    }
    
    int pop()
    {
        int ret = last(); 
        removeat(count - 1); 
        return ret; 
    }
    
    int dequeue()
    {
        int ret = first(); 
        removeat(0); 
        return ret; 
    }
    
    int first()
    {
        return buffer[0]; 
    }
    
    int last()
    {
        return buffer[count - 1]; 
    }
    
    void destroy()
    {
        free(buffer); 
    }
    
    void add(int value)
    {
        if (count >= capacity)
        {
            capacity <<= 1; 
            buffer = RALLOC(buffer, int, capacity); 
        }
        buffer[count++] = value; 
    }
    
    void removeat(u32 index)
    {
        for (u32 i = index, cnt = count - index - 1; cnt > 0; i++, cnt--)
        buffer[i] = buffer[i + 1]; 
        if (clrfun) clrfun(&buffer[count - 1]); 
        count--; 
    }
    
    s32 find(int value)
    {
        FOR(count)
            if (cmpfun(buffer[i], value))
                return i;
        return -1;
    }
    
    void remove(int value)
    {
        s32 index = find(value); 
        if (index == -1)
        return; 
        removeat(index); 
    }
    
    void clear()
    {
        if (clrfun)
        FOR(count)
        clrfun(&buffer[i]); 
        count = 0; 
    }
    
    int& at(u32 index)
    {
        return buffer[index]; 
    }
    
    int& operator [](u32 index)
    {
        return buffer[index]; 
    }
};

struct LIST_float
{
    u32     count; 
    u32     capacity; 
    float*     buffer; 
    void    (*clrfun)(float* element); 
    s32     (*cmpfun)(float x, float y); 
    
    LIST_float(u32 capacity, 
            void(*clrfun)(float*) = null, 
            s32(*cmpfun)(float, float) = null)
    {
        this->count = 0; 
        this->capacity = capacity; 
        this->buffer = CALLOC(float, capacity); 
        this->clrfun = clrfun; 
        this->cmpfun = cmpfun; 
    }
    
    bool empty()
    {
        return count == 0; 
    }
    
    float pop()
    {
        float ret = last(); 
        removeat(count - 1); 
        return ret; 
    }
    
    float dequeue()
    {
        float ret = first(); 
        removeat(0); 
        return ret; 
    }
    
    float first()
    {
        return buffer[0]; 
    }
    
    float last()
    {
        return buffer[count - 1]; 
    }
    
    void destroy()
    {
        free(buffer); 
    }
    
    void add(float value)
    {
        if (count >= capacity)
        {
            capacity <<= 1; 
            buffer = RALLOC(buffer, float, capacity); 
        }
        buffer[count++] = value; 
    }
    
    void removeat(u32 index)
    {
        for (u32 i = index, cnt = count - index - 1; cnt > 0; i++, cnt--)
        buffer[i] = buffer[i + 1]; 
        if (clrfun) clrfun(&buffer[count - 1]); 
        count--; 
    }
    
    s32 find(float value)
    {
        FOR(count)
            if (cmpfun(buffer[i], value))
                return i;
        return -1;
    }
    
    void remove(float value)
    {
        s32 index = find(value); 
        if (index == -1)
        return; 
        removeat(index); 
    }
    
    void clear()
    {
        if (clrfun)
        FOR(count)
        clrfun(&buffer[i]); 
        count = 0; 
    }
    
    float& at(u32 index)
    {
        return buffer[index]; 
    }
    
    float& operator [](u32 index)
    {
        return buffer[index]; 
    }
};

struct ARRAY_string
{
    string*   buffer; 
    u32  count; 
    
    ARRAY_string(string* initvalue, u32 initcount)
    {
        buffer = initvalue; 
        count = initcount; 
    }
    
    ARRAY_string(u32 initcount)
    {
        buffer = CALLOC(string, initcount); 
        count = initcount; 
    }
    
    string& operator [] (u32 index)
    {
        return buffer[index]; 
    }
};

struct ARRAY_int
{
    int*   buffer; 
    u32  count; 

    ARRAY_int()
    {
        buffer = CALLOC(int, 4);
    }
    
    ARRAY_int(int* initvalue, u32 initcount)
    {
        buffer = initvalue; 
        count = initcount; 
    }
    
    ARRAY_int(u32 initcount)
    {
        buffer = CALLOC(int, initcount); 
        count = initcount; 
    }
    
    int& operator [] (u32 index)
    {
        return buffer[index]; 
    }
};

struct ARRAY_float
{
    float*   buffer; 
    u32  count; 
    
    ARRAY_float(float* initvalue, u32 initcount)
    {
        buffer = initvalue; 
        count = initcount; 
    }
    
    ARRAY_float(u32 initcount)
    {
        buffer = CALLOC(float, initcount); 
        count = initcount; 
    }
    
    float& operator [] (u32 index)
    {
        return buffer[index]; 
    }
};


struct v3_string_int_float
{
    string x;
    int y;
    float z;
};



struct v3_u32_float_double
{
    u32 x;
    float y;
    double z;
};


