#define N_TYPES 64
#define N_FUNCS 128
#define N_FIELDS 32
#define N_PARAMS 16
#define N_NOTE_ARGS 32
#define N_NOTES 32
#define meta(...)

#define MetaId(Container, Value) STRING_SEARCH(Container.Names, Container.Count, Value)
#define MetaTypeId(Name) MetaId(MetaTypes, Name)
#define MetaAlloc(T) MetaTypes.Allocs[MetaId(MetaTypes, T)]()
#define MetaCall(Name, ret, ...) MetaFuncs.Calls[MetaId(MetaFuncs, Name)](ret, 0, __VA_ARGS__)
#define MetaSetGlobal(Name, Value) MetaTypes.Fields[0].Setters[MetaId(MetaTypes.Fields[0], Name)](null, Value)
#define MetaGetGlobal(Name, T) *(T*)MetaTypes.fields[0].Getters[MetaId(MetaTypes.Fields[0], Name)](null)

#define MetaSetMember(Target, Name, Value)\
    MetaTypes.Fields[TypeId(Target)].Setters[MetaId(MetaTypes.Fields[TypeId(Target)], Name)](Target, Value)
#define MetaGetMember(Target, Name)\
    MetaTypes.Fields[TypeId(Target)].Getters[MetaId(MetaTypes.Fields[TypeId(Target)], Name)](Target)

typedef void  (*meta_call)(void* ret, ...);
typedef void* (*meta_alloc)();
typedef void  (*meta_set)(void* Target, void* Value);
typedef void* (*meta_get)(void* Target);

enum { INT, FLOAT, BOOL, STRING };

struct meta_note_value
{
    float f;
    string s;
    u32 type;

    meta_note_value(const float& Value)
    {
        f = Value;
        type = FLOAT;
        s = SALLOC(32);
        _snprintf(s, 32, "%f", Value);
    }

    meta_note_value(const int& Value)   
    {
        f = (float)Value;
        type = INT;
        s = SALLOC(32);
        sprintf(s, "%d", Value);
    }

    meta_note_value(const bool& Value)  
    {
        f = Value;
        type = BOOL;
        s = Value ? "true" : "false";
    }

    meta_note_value(const string& Value)
    {
        s = Value;
        type = STRING;
    }

    meta_note_value()                   
    {
        f = 0;
        type = INT;
    }

    operator float()  { return f; }
    operator string() { return s; }
    operator int()    { return (int)f; }
    operator bool()   { return (int)f & 1; } // & 1 to hide warning

    bool operator < (float Value) { return f < Value; }
    bool operator < (int   Value) { return f < Value; }
    bool operator > (float Value) { return f > Value; }
    bool operator > (int   Value) { return f > Value; }
    bool operator == (string Value) { return streql(s, Value); }
    // etc...
};

static meta_note_value InvalidNoteValue = -1;

struct meta_note_params
{
    string          Keys[N_NOTE_ARGS];
    meta_note_value Values[N_NOTE_ARGS];
    u32             Count;

    void Add(string Key, meta_note_value Value)
    {
        Assert(Count < N_NOTE_ARGS);
        Keys[Count] = Key;
        Values[Count++] = Value;
    }

    meta_note_value& operator () (u32 Index)
    {
        Assert(Index < Count);
        return Values[Index];
    }

    meta_note_value& operator () (string Key, meta_note_value& Default)
    {
        return ValueOrDefault(Key, Default);
    }

    meta_note_value& operator () (string Key)
    {
        return ValueOrDefault(Key, InvalidNoteValue);
    }

    meta_note_value& ValueOrDefault(string Key, meta_note_value& Default)
    {
        s32 i = STRING_SEARCH(Keys, Count, Key);
        if (i == -1)
            return Default;
        return Values[i];
    }
};

struct meta_notes
{
    u32                Count;
    string             Names[N_NOTES];
    meta_note_params   Params[N_NOTES];
};

struct meta_fields
{
    u32         Count;
    string      Types[N_FIELDS];
    string      Names[N_FIELDS];
    meta_notes  Notes[N_FIELDS];
    meta_set    Setters[N_FIELDS];
    meta_get    Getters[N_FIELDS];
};

struct meta_funcs
{
    u32          Count;
    string       Types[N_FUNCS];
    string       Names[N_FUNCS];
    meta_notes   Notes[N_FUNCS];
    meta_fields  Params[N_FUNCS];
    meta_call    Calls[N_FUNCS];
} MetaFuncs;

struct meta_types // SOA struct of array, AOS Array of struct
{
    u32          Count;
    string       Names[N_TYPES];
    meta_notes   Notes[N_TYPES];
    meta_fields  Fields[N_TYPES];
    meta_alloc   Allocs[N_TYPES];
} MetaTypes;

// to be redefined in the generated meta to point to the meta initialization function
#define META_INIT()

