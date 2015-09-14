
#define N_TYPES 64
#define N_FUNCS 128
#define N_FIELDS 32
#define N_PARAMS 16
#define N_NOTE_ARGS 32
#define N_NOTES 32
#define meta(...)

#define META_ID(container, value) LINEAR_SEARCH(string, container.names, container.count, value, cmp_string)
#define META_TYPE_ID(name) META_ID(metatypes, name)
#define META_ALLOC(T) metatypes.allocs[META_ID(metatypes, T)]()
#define META_CALL(name, ret, ...) metafuncs.calls[META_ID(metafuncs, name)](ret, 0, __VA_ARGS__)
#define META_SET_GLOBAL(name, value) metafields.sets[META_ID(metafields, name)](null, value)
#define META_GET_GLOBAL(name, T) (*(T*)metafields.gets[META_ID(metafields, name)](null))
#define META_SET_MEMBER(target, name, value) metatypes.fields[type_id(target)].sets[META_ID(metatypes.fields[type_id(target)], name)](target, value)
#define META_GET_MEMBER(target, name) metatypes.fields[type_id(target)].gets[META_ID(metatypes.fields[type_id(target)], name)](target)

typedef void  (*mt_call)(void* ret, u8 start, ...);
typedef void* (*mt_alloc)();
typedef void  (*mt_set)(void* target, void* value);
typedef void* (*mt_get)(void* target);

union mt_note_value
{
    float f;
    string s;

    mt_note_value(const float& value)  { f = value; }
    mt_note_value(const int& value)    { f = (float)value; }
    mt_note_value(const bool& value)   { f = value; }
    mt_note_value(const string& value) { s = value; }
    mt_note_value()                    { f = 0;     }

    operator float()  { return f; }
    operator string() { return s; }
    operator int()    { return (int)f; }
    operator bool()   { return (int)f & 1; } // & 1 to hide warning

    // weird C++ syntax to indicate that this is a postfix and not prefix
    mt_note_value& operator ++ (int) { f++; return *this; }
    mt_note_value& operator ++ () { f++; return *this; }

    mt_note_value& operator += (float value) { f += value; return *this; }
    mt_note_value& operator += (int value) { f += value; return *this; }

    bool operator < (float value) { return f < value; }
    bool operator < (int   value) { return f < value; }
    bool operator > (float value) { return f > value; }
    bool operator > (int   value) { return f > value; }

    mt_note_value& operator + (const mt_note_value& other) { f += other.f; return *this; }
    mt_note_value& operator - (const mt_note_value& other) { f -= other.f; return *this; }
};

struct mt_note_params
{
    string         keys[N_NOTE_ARGS];
    mt_note_value  values[N_NOTE_ARGS];
    u32            count;

    static mt_note_value invalid; // TODO assign to -1

    void add(string key, mt_note_value value)
    {
        assert(count < N_NOTE_ARGS);
        keys[count] = key;
        values[count++] = value;
    }

    mt_note_value& operator () (u32 idx)
    {
        assert(idx < count);
        return values[idx];
    }

    mt_note_value& operator () (string key, mt_note_value& default)
    {
        return value_or_default(key, default);
    }

    mt_note_value& operator () (string key)
    {
        return value_or_default(key, invalid);
    }

    mt_note_value& value_or_default(string key, mt_note_value& default)
    {
        s32 i = STRING_SEARCH(keys, count, key);
        if (i == -1)
            return default;
        return values[i];
    }
};

struct mt_notes
{
    u32              count;
    string           names[N_NOTES];
    mt_note_params   params[N_NOTES];
};

struct mt_fields
{
    u32       count;
    string    types[N_FIELDS];
    string    names[N_FIELDS];
    mt_notes  notes[N_FIELDS];
    mt_set    sets[N_FIELDS];
    mt_get    gets[N_FIELDS];
} metafields;

struct mt_funcs
{
    u32        count;
    string     types[N_FUNCS];
    string     names[N_FUNCS];
    mt_notes   notes[N_FUNCS];
    mt_fields  params[N_FUNCS];
    mt_call    calls[N_FUNCS];
} metafuncs;

struct mt_types // SOA struct of array, AOS Array of struct
{
    u32        count;
    string     names[N_TYPES];
    mt_notes   notes[N_TYPES];
    mt_fields  fields[N_TYPES];
    mt_alloc   allocs[N_TYPES];
} metatypes;

// to be redefined in the generated meta to point to the meta initialization function
#define META_INIT()

