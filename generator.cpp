#include "common.h"

/*
    meta generation limitations:
    - [parser] any * or & in function parameter or field must be accompanied with type and not the parameter/variable name
    - [parser] feild assignments are not ignored yet and will confuse the parser
    - [compiling] META_INIT_DONE hack
*/

#define MAX_FUN_ARGS 32

struct NoteParam
{
    string keys[32];
    string values[32];
    u8     count;

    void add(string key, string value)
    {
        assert(count < 32);
        keys[count] = key;
        values[count++] = value;
    }
};

struct Notes
{
    string      names[32];
    NoteParam   params[32];
    u32         count;
    
} notes;

void eatf(string& s)
{
    while(isspace(*s))
        s++;
}

void eatb(string& s)
{
    u32 i = strlen(s) - 1;
    while(isspace(s[i]) && i >= 0)
    {
        s[i] = '\0';
        i--;
    }
}

void eat(string& s)
{
    eatf(s);
    eatb(s);
}

inline string read(string& s, char end0, char end1, bool stopspace = true)
{
    string result = MALLOCX(char, 256);
    u32 i;
    
    eatf(s);
    for(i = 0; *s && (!stopspace || !isspace(*s)) && *s != end0 && *s != end1; i++)
        result[i] = *s++;
    result[i] = '\0';
    eatf(s);
    
    return result;
}

string read(string& s, char end0)
{
    return read(s, end0, '\0');
}

string read(string& s)
{
    return read(s, '\0');
}

bool expect(string& s, string value)
{
    eatf(s);
    bool ret = *s == *value;
    return ret;
}

void gen_meta(string outputpath, u32 numfiles, string* infiles)
{
    FILE* metafile = fopen(outputpath, "w");
    if(metafile == null)
    {
        fprintf(stderr, "Error: Couldn't open output metafile '%s'", outputpath);
        return;
    }
    
    u32 ntypes = 0, nfuncs = 0, nglobals = 0;
    
    #define MB (1024LL * 1024LL)
    string out_metatypes    = SALLOC(1 * MB),
           out_metafuncs    = SALLOC(2 * MB),
           out_metafields   = SALLOC(1 * MB),
           out_typeallocs   = SALLOC(1 * MB),
           out_funcalls     = SALLOC(2 * MB),
           out_typeids      = SALLOC(8 * MB),
           out_globalgetset = SALLOC(1 * MB),
           out_instgetset   = SALLOC(3 * MB);
    
    #define W(out, line, ...) sprintf((out) + strlen((out)), line, __VA_ARGS__)
    #define WL(out, line, ...) W(out, line"\n", __VA_ARGS__)
    #define WI(out, line, ...) W(out, "\t"line, __VA_ARGS__)
    #define WIL(out, line, ...) WI(out, line"\n", __VA_ARGS__)
    
    FUNC(void, write_notes, (string out, string target, u32 targetid),
    {
        WIL(out, "%s.notes[%d].count = %d;",
                target, targetid, notes.count);
        
        FOR(notes.count)
        {
            WIL(out, "%s.notes[%d].names[%d] = \"%s\";",
                    target, targetid, i, notes.names[i]);

            NoteParam& p = notes.params[i];
            FORk(p.count)
            {
                WIL(out, "%s.notes[%d].params[%d].add(\"%s\", %s);",
                        target, targetid, k, p.keys[k], p.values[k]);
            }
        }
    });
    
    FOR(numfiles)
    {
        
        string filepath = infiles[i];
        string s = read_text_file(filepath);
        if (!s)
        {
            fprintf(stderr, "Error: Couldn't read input file content for parsing '%s'", filepath);
            return;
        }
        
        while(*s)
        {
            string token = read(s, ' ', '(');
            if (!streql(token, "meta"))
            {
                if (*s == '(')
                    s++;
                continue;
            }

            // Is the word 'meta' mentioned in a string or comment? ignore.
            // This assumes that the user will always use the form 'meta()'
            // when he's actually annotating stuff with it.
            // There is no handling of 'meta()' inside comments/strings
            if (!expect(s, "("))
            {
                s++;
                continue;
            }

            eatf(++s);
            bool note = *s != ')';
            if (!note)
            {
                eat(++s);
            }
            else
            {
                // meta(helpbox["message...", comment = true], popup["getvalues"])
                u32& nc = notes.count;
                FOR(nc) notes.params[i].count = 0;
                nc = 0;
                while(1)
                {

                    string notename = read(s, '[');
                    notes.names[nc] = notename;
                    //WIL(out_metafields, "metafields.notes[%d].names[%d] = \"%s\";",
                    //nglobals, nc, notename);

                    eatf(++s);

                    // parse note key/value
                    while(1)
                    {
                        string noteparam = read(s, ',', ']', false);

                        string value = strchr(noteparam, '=');
                        assert(value != null, "note `%s` parameter missing '=' sign", notename);

                        ptrdiff_t eqlidx = value - noteparam;
                        assert(eqlidx > 0);
                        eatf(++value); // eat = and space
                        printf("note %s value=%s\n", notename, value);

                        string key = noteparam;
                        key[eqlidx] = '\0';
                        eatb(key);
                        printf("note %s key=%s\n", notename, key);

                        notes.params[nc].add(key, value);
                        //WIL(out_metafields, "metafields.notes[%d].params[%d].add(\"%s\", %s);",
                        //nglobals, nc, key, value);

                        if (*s == ',')
                            eatf(++s);
                        else
                        {
                            // end of note
                            if (*s != ']')
                            {
                                fprintf(stderr, "Unexpected end of note `%s` in file `%s`\n", notename, filepath);
                                return;
                            }
                            nc++;
                            eatf(++s);
                            break;
                        }
                    }

                    if (*s == ',')
                        s++;
                    else // end of all notes
                    {
                        eatf(++s); // eat )
                        //WIL(out_metafields, "metafields.notes[%d].count = %d;",
                        //nglobals, nc);
                        break;
                    }
                }
            }

            token = read(s);
            if (streql(token, "struct"))
            {
                u32 nfields = 0;
                string tpname = read(s);

                WIL(out_metatypes, "metatypes.names[%d] = \"%s\";",
                        ntypes, tpname);

                eatf(s);
                assert(*s == '{');
                s++;

                while(*s != '}')
                {
                    string fieldtype = read(s);

                    while(1)
                    {
                        string fieldname = read(s, ',', ';');

                        // write struct fields
                        WIL(out_metatypes, "metatypes.fields[%d].types[%d] = \"%s\";",
                                ntypes, nfields, fieldtype);
                        WIL(out_metatypes, "metatypes.fields[%d].names[%d] = \"%s\";",
                                ntypes, nfields, fieldname);

                        // write struct fields get/set functions
                        WL(out_instgetset,
                                "inline void* get_%s_%s(void* target) { return &((%s*)target)->%s; }",
                                tpname, fieldname, tpname, fieldname);
                        WL(out_instgetset,
                                "inline void set_%s_%s(void* target, void* value) { ((%s*)target)->%s = *(%s*)value; }",
                                tpname, fieldname, tpname, fieldname, fieldtype);

                        // write struct field get/set bindings
                        WIL(out_metatypes, "metatypes.fields[%d].gets[%d] = get_%s_%s;",
                                ntypes, nfields, tpname, fieldname);
                        WIL(out_metatypes, "metatypes.fields[%d].sets[%d] = set_%s_%s;",
                                ntypes, nfields, tpname, fieldname);

                        nfields++;

                        if (*s == ',')
                            s++;
                        else if (*s == ';')
                        {
                            s++; // eat ';'
                            eatf(s);
                            break;
                        }
                    }
                }

                // skip any instance declarations
                while(*s++ != ';');

                // write struct number of fields
                WIL(out_metatypes, "metatypes.fields[%d].count = %d;",
                        ntypes, nfields);

                // write type alloc
                WL(out_typeallocs, "inline void* alloc_%s() { return MALLOC(%s); }",
                        tpname, tpname);

                // bind alloc func ptr
                WIL(out_metatypes, "metatypes.allocs[%d] = alloc_%s;",
                        ntypes, tpname);

                // write type-id binding
                WL(out_typeids, "inline u32 type_id(%s* ptr) { return %d; }",
                        tpname, ntypes);

                // write type notes if any
                if (note)
                    write_notes(out_metatypes, "metatypes", ntypes);
                
                ntypes++;
            }

            else
            {
                string tmp = s;
                while(*tmp != ';' && *tmp != '(') tmp++;
                if (*tmp == ';') // global variable?
                {
                    string fieldtype = token;
                    while(1)
                    {
                        string fieldname = read(s, ',', ';');

                        // generate global field info
                        WIL(out_metafields, "metafields.names[%d] = \"%s\";",
                                nglobals, fieldname);
                        WIL(out_metafields, "metafields.types[%d] = \"%s\";",
                                nglobals, fieldtype);

                        // write global field getter/setter functions
                        WL(out_globalgetset,
                                "inline void* get_%s(void* target) { return &%s; }",
                                fieldname, fieldname);
                        WL(out_globalgetset,
                                "inline void set_%s(void* target, void* value) { %s = *(%s*)value; }",
                                fieldname, fieldname, fieldtype);

                        // write global field getter/setter bindings
                        WIL(out_metafields, "metafields.gets[%d] = get_%s;",
                                nglobals, fieldname);
                        WIL(out_metafields, "metafields.sets[%d] = set_%s;",
                                nglobals, fieldname);

                        // write global field notes if any
                        if (note)
                            write_notes(out_metafields, "metafields", nglobals);

                        nglobals++;

                        if (*s == ',') s++;
                        else if (*s == ';') break;
                    }
                    s++; // eat ';'
                }

                else // function!
                {
                    string funcname = read(s, '(');
                    string functype = token;

                    // write metafuncs type and name
                    WIL(out_metafuncs, "metafuncs.types[%d] = \"%s\";",
                            nfuncs, functype);
                    WIL(out_metafuncs, "metafuncs.names[%d] = \"%s\";",
                            nfuncs, funcname);

                    s++; // eat (
                    eatf(s);
                    if (*s == ')') // has no params?
                    {
                        WIL(out_metafuncs, "metafuncs.params[%d].count = 0;",
                                nfuncs);

                        // write paramless funcall
                        WL(out_funcalls,
                                "inline void call_%s(void* ret, u8 start, ...) { %s(); }",
                                funcname, funcname);

                        // write paramless call bindings
                        WIL(out_metafuncs, "metafuncs.calls[%d] = call_%s;",
                                nfuncs, funcname);
                    }
                    else
                    {
                        u32 nparams = 0;

                        W(out_funcalls, "inline void call_%s(void* ret, u8 start, ...)\n"
                                "{\n" "\tva_begin(start);" "\n",
                                funcname, funcname);

                        string pnames[MAX_FUN_ARGS];
                        
                        while(*s != ')')
                        {
                            string paramtype = read(s);
                            string paramname = read(s, ',', ')');

                            if (*s == ',')
                                s++;

                            pnames[nparams] = paramname;

                            // write param info
                            WIL(out_metafuncs, "metafuncs.params[%d].types[%d] = \"%s\";",
                                    nfuncs, nparams, paramtype);
                            WIL(out_metafuncs, "metafuncs.params[%d].names[%d] = \"%s\";",
                                    nfuncs, nparams, paramname);

                            // fetch param from va_arg
                            WIL(out_funcalls, "%s %s = va_arg(args, %s);",
                                    paramtype, paramname, paramtype);

                            nparams++;
                        }

                        // write funcall invocation
                        if (streql(functype, "void"))
                            W(out_funcalls, "\t" "%s(", funcname);
                        else
                            W(out_funcalls, "\t" "*(%s*)ret = %s(",
                                    functype, funcname);

                        FOR(nparams-1)
                            W(out_funcalls, "%s, ", pnames[i]);
                        WL(out_funcalls, "%s);", pnames[nparams-1]);
                        
                        // write funcall tail 
                        WL(out_funcalls, "\t" "va_end(args);" "\n}");

                        // write call bindings
                        WIL(out_metafuncs, "metafuncs.calls[%d] = call_%s;",
                                nfuncs, funcname);

                        // write params count
                        WIL(out_metafuncs, "metafuncs.params[%d].count = %d;",
                                nfuncs, nparams);

                    }
                    s++; // eat )
                    
                    // write function notes if any
                    if (note)
                        write_notes(out_metafuncs, "metafuncs", nfuncs);

                    nfuncs++;
                }
            }
            eatf(s);
        }
    }

    WIL(out_metatypes,  "metatypes.count = %d;\n",  ntypes);
    WIL(out_metafuncs,  "metafuncs.count = %d;\n",  nfuncs);
    WIL(out_metafields, "metafields.count = %d;\n", nglobals);

    //WL(out_typeids, "inline u32 type_id(void* ptr) { return assert(0); }");

    #define fwcomment(file, comment, block)\
        if (strlen(block) > 0) fprintf(file, "\n// "comment"\n%s", block)
    
    fwcomment(metafile, "Type allocators", out_typeallocs);
    fwcomment(metafile, "Function callers", out_funcalls);
    fwcomment(metafile, "Type-id bindings", out_typeids);
    fwcomment(metafile, "Global variables [g|s]etters", out_globalgetset);
    fwcomment(metafile, "Instance fields [g|s]etters", out_instgetset);
    
    fprintf(metafile, "\n// Meta-data initialization\n");
    fprintf(metafile, "void meta_init()\n{\n" "%s%s%s" "}\n", out_metatypes, out_metafuncs, out_metafields);
    fprintf(metafile, "#undef META_INIT\n");
    fprintf(metafile, "#define META_INIT() meta_init()\n");
   
    fclose(metafile);

    #undef W
    #undef WL
    #undef WI
    #undef WIL
    #undef fwcomment
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

        result[next] = SALLOC(split_length + 1);
        memcpy(result[next], s, split_length);
        result[next][split_length] = '\0';
        eat(result[next]);
        s += split_length + (next < finds);
        next++;
    }

    return finds + 1;
}

void gen_template(string out_path, int nargs, string* args)
{
    FILE* out_file = fopen(out_path, "wb");
    if (!out_file)
    {
        fprintf(stderr, "failed to open template output %s", out_path);
        return;
    }

    for(int x = 0; x < nargs; x += 2)
    {
        string in_file = args[x];
        string content = read_text_file(in_file);
        if (!content)
        {
            fprintf(stderr, "failed to read file %s\n", in_file);
            continue;
        }

        string* split_by_semicolin;
        string* split_by_comma;
        string* split_by_equal;

        u32 n_splits_semicolin;
        u32 n_splits_comma;
        u32 n_splits_equal;
        
        string params = args[x + 1]; // e.g. "X=string,Y=int,Z=float;X=u32,Y=float,Z=double";
        n_splits_semicolin = strsplit(params, ';', split_by_semicolin);

        FORi(n_splits_semicolin)
        {
            string s = content;
            string semicolin_split = split_by_semicolin[i];
            n_splits_comma = strsplit(semicolin_split, ',', split_by_comma);

            assert(n_splits_comma <= 8, "NOT SUPPORTED! "
                                        "Why on earth would you have more than 8 type parameters? "
                                        "Consider re-evaluating your design.");
            char keys[8];
            string values[8];

            u32 kidx = 0;
            u32 vidx = 0;

            FORj(n_splits_comma)
            {
                string comma_split = split_by_comma[j];
                n_splits_equal = strsplit(comma_split, '=', split_by_equal);
                assert(n_splits_equal == 2, "There should have been only one assignment operator. "
                                            "Multiple assignments are not supported.");
                keys[kidx++] = split_by_equal[0][0];
                values[vidx++] = split_by_equal[1];
            }

            while(*s)
            {
                bool replaced = false;
                FORk(n_splits_comma)
                {
                    if (s[0] == '$' && s[1] == keys[k])
                    {
                        fprintf(out_file, values[k]);
                        s += 2; // assuming the user will always use $T, $K, $V, etc
                        replaced = true;
                        break;
                    }
                }
                if (!replaced)
                {
                    fprintf(out_file, "%c", *s);
                    s++;
                }
            }
        }

    }
}

void test(string* &ptr)
{
    ptr = CALLOC(string, 2);
    ptr[0] = "Hello";
    ptr[1] = "World";
}

int main(int argc, string argv[])
{
    // gen -t out.gen in0.h/cpp in1.h/cpp etc
    // gen -m out.gen in0.h/cpp in1.h/cpp etc
    // gen -s out.gen in0.h/cpp in1.h/cpp etc

#if 0
    if (argc < 4)
    {
        puts("gen must be called with at least 3 parameters:\n"
            "mode: -t (type generation) -m (meta generation) -s (serialization generation)\n"
            "output file: path to the file to generate to\n"
            "input files: at least one input file\n");
        return -1;
    }

    string mode = argv[1];

    if (streql(mode, "-t"))
    {
        if (argc < 5)
        {
            puts("gen -t must be called with at least 3 parameters:\n"
                    "output file: path to the file to generate to\n"
                    "input file: input file\n"
                    "from:to type transformation: specifies what the type symbol to replace with what.\n"
                    "e.g. gen -t out.cpp list.temp T:int dict.temp K:string,V:some_type");
            return -1;
        }

        string out_file = argv[2];
        string* infiles = argv + 3;
        u32 nargs = (argc - 3) / 2;

        gen_template(out_file, nargs, infiles);
    }
    else if (streql(mode, "-m"))
    {
        string out_file = argv[2];
        string* infiles = argv + 3;
        u32 nargs = argc - 3;

        gen_meta(out_file, nargs, infiles);
    }
    else if (streql(mode, "-s"))
    {
    }
    return 0;
#elif 0
    string* infiles = CALLOC(string, 32);
    u32 n = 0;
    infiles[n++] = "../list.template";
    infiles[n++] = "T=string;T=int;T=float";
    infiles[n++] = "../array.template";
    infiles[n++] = "T=string;T=int;T=float";
    infiles[n++] = "../test.template";
    infiles[n++] = "X=string,Y=int,Z=float;X=u32,Y=float,Z=double";
    gen_template("../collections.cpp", n, infiles);
#else
    string out_file = "../meta.gen";
    u32 n = 0;
    string* infiles = CALLOC(string, 32);
    infiles[n++] = "../Test1.cpp";
    infiles[n++] = "../Test0.cpp";

    gen_meta(out_file, n, infiles);
#endif

}
