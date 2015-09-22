
//Vim keep yanked word after delete
//Vim indent block

#include "common.h"
#include "meta.h"
#include "lab.h"

#include "Test0.cpp"
#include "Test1.cpp"

#include "meta.gen"
#include "collections.cpp"

#define LIST(T) LIST_##T
#define ARRAY(T) ARRAY_##T

#define META_GET_TYPE_NOTES(T) MetaTypes.Notes[MetaId(MetaTypes, T)]
#define META_GET_TYPE_FIELDS(T) MetaTypes.fields[MetaId(MetaTypes, T)]

#define dyn_free(a)         ((a) ? free(_dyn_baseptr(a)), 1 : 0)
#define dyn_push(T, a, x)   (_dyn_checkgrow(T, a, 1), (a)[_dyn_base1(a)++] = x)
#define dyn_count(a)        ((a) ? _dyn_base1(a) : 0)
#define dyn_last(a)         ((a)[dyn_count(a) - 1])
#define dyn_add(T, a, n)    (_dyn_checkgrow(T, a, n), _dyn_base1(a) += (n), &(a)[dyn_count(a) - (n)])
#define dyn_reset(a)        (a ? _dyn_base1(a) = 0 : 0)

#define _dyn_baseptr(a)         ((int*)(a) - 2)
#define _dyn_base0(a)           (_dyn_baseptr(a)[0])
#define _dyn_base1(a)           (_dyn_baseptr(a)[1])
#define _dyn_capacity(a)        ((a) ? _dyn_base0(a) : 0)
#define _dyn_checkgrow(T, a, n) (((a) == NULL || dyn_count(a) + (n) >= _dyn_capacity(a)) ? \
                                ((a) = (T*)_dyn_grow((a), (n), sizeof(*a)), 1) : 0)

static void* _dyn_grow(void* a, int increment, int itemsize)
{
    int* base;

    if (a)
    {
        // calculate how much bytes to allocate
        // we use whatever is greater: our count + increment, or double capacity
        int need = _dyn_base1(a) + increment;
        int dbl_cap = _dyn_capacity(a) * 2;
        int newcap = need > dbl_cap ? need : dbl_cap;
        base = (int *)realloc(_dyn_baseptr(a), 2 * sizeof(int) + itemsize * newcap);
        //assert0(base, "Out of memory. Not good!");
        base[0] = newcap;
    }
    else
    {
        int newcap = increment < 4 ? 4 : increment;
        base = (int*)realloc(0, 2 * sizeof(int) + itemsize * newcap);
        //assert0(base, "Out of memory. Not good!");
        base[0] = newcap;
        base[1] = 0;
    }

    return base + 2;
}

//#define ListFree(x)     ((x) ? free(_ListBase(x)), 1 : 0)
//#define ListCount(x)    ((x) ? _ListHeader1(x) : 0)
//#define ListCapacity(x) ((x) ? _ListHeader0(x) : 0)
//#define ListPush(x, v)  (_ListCheckGrow(x, 1), (x)[_ListHeader1(x)++] = v)
//#define ListAdd(x, n)   (_ListCheckGrow(x, n), _ListHeader1(x) += (n), &(x)[_ListHeader1(x) - (n)])

//#define _ListBase(x)    ((int*)(x) - 2)
//#define _ListHeader0(x) (_ListBase(x)[0])
//#define _ListHeader1(x) (_ListBase(x)[1])
//#define _ListCheckGrow(x, n) ((x == null || _ListHeader1(x) + (n) > _ListHeader0(x)) ?
                              //(x = ListGrow((x), (n), sizeof(*(x))), 1) : 0)

//static void* ListGrow(void* List, u32 Increment, u32 ItemSize)
//{
    //int* Ptr;
    //if (List)
    //{
        //u32 DoubleCapacity = _ListHeader0(List) * 2;
        //u32 Need = _ListHeader1(List) + Increment;
        //u32 NewCapacity = DoubleCapacity > Need ? DoubleCapacity : Need;
        //Ptr = (int*)realloc(_ListBase(List), sizeof(int) * 2 + NewCapacity * ItemSize);
        //Assert(Ptr);
        //Ptr[0] = NewCapacity;
    //}
    //else
    //{
        //u32 NewCapacity = Increment < 4 ? 4 : Increment;
        //Ptr = malloc(sizeof(int) * 2 + NewCapacity * ItemSize);
        //Ptr[0] = Increment;
        //Ptr[1] = 0;
    //}
    //return Ptr + 2;
//}

int main()
{
    struct function_declaration
    {
        char* Type;
        char* Name;
        StaticArray(string, ParamNames, 32);
        StaticArray(string, ParamTypes, 32);
        //string ParamNames[32];
        //u32 ParamNamesCount;
        //string ParamTypes[32];
        //u32 ParamTypesCount;

        //function_declaration()
        //{
            //Type = null;
            //Name = null;
            //ParamNamesCount = 0;
            //ParamTypesCount = 0;
            //ArrayZero(ParamTypes, 32);
            //ArrayZero(ParamNames, 32);
        //}
    };

    function_declaration fd = {};
    printf("", fd);

    META_INIT();

#if 1
#elif 0

    string tpname = "TestStruct";
    meta_notes& test_notes = META_GET_TYPE_NOTES(tpname);
    FORi(test_notes.Count)
    {
        printf("@%s: %s\n", tpname, test_notes.names[i]);
        meta_note_params& params = test_notes.params[i];
        FORj(params.Count)
            printf("param[%d]: Key=%s, Value=%s\n", i, params.Keys[j], (string)params.Values[j]);
    }

    meta_fields& Fields = META_GET_TYPE_FIELDS(tpname);
    FORj(fields.Count)
    {
        printf("Field[%d]: %s %s\n", j, Fields.types[j], Fields.Names[j]);
        meta_notes& field_notes = Fields.Notes[j];
        FORi(field_notes.Count)
        {
            meta_note_params& params = field_notes.params[i];
            FORk(params.Count)
                printf("param[%d]: Key=%s, Value=%s\n", k, params.Keys[k], (string)params.Values[k]);
        }
    }

    getchar();
#elif 0
    // get/set global fields dynamically
    int ten = 10;
    META_SET_GLOBAL("result", &ten);
    int back = META_GET_GLOBAL("result", int);

    // invoke function dynamically
    int five;
    META_CALL("sum", &five, 2, 3);

    // create objects dynamically
    Player* p = (Player*)MetaAlloc("Player");

    // set/get fields dynamically
    META_SET_MEMBER(p, "id", &five);
    int id = *(int*)META_GET_MEMBER(p, "id");

    // iterate over a type fields and view their Name and type
    u32 pid = TypeId(p); // or MetaTypeId("Player")
    meta_fields& player_fields = MetaTypes.Fields[pid];
    FOR(player_fields.Count)
        printf("%s Player.%s\n", player_fields.types[i], player_fields.Names[i]);

    puts("");

    //// iterate over global fields
    //FOR(metafields.Count)
        //printf("%s %s\n", metafields.types[i], metafields.Names[i]);

    puts("");

    getchar();
#endif

    //// iterate over functions and read their parameters
    //FOR(MetaFuncs.Count)
    //{
        //printf("%s (", MetaFuncs.Names[i]);
        
        //meta_fields& params = MetaFuncs.params[i];
        //FOR(params.Count)
            //printf("%d: %s %s ", i, params.types[i], params.Names[i]);
        
        //puts(")");
    //}

    //getchar();
    //return;
    
    //int array[] = { 1, 2, 3, 4, 5, 4, 2, 14, 200, 4 };

    //FUNC(bool, iseven, (void* n),
            //{ return (u32)n % 2 == 0; });

    //FUNC(s8, cmp, (void* x, void* y),
            //{ return (u32)x - (u32)y > (u32)y - (u32)x; });

    //FUNC(void*, tostr, (void* n),
            //{ char* buffer = MALLOCX(char, 2); itoa((int)n, buffer, 10); return buffer; });
    
    //linq_from(array, 10, cmp)
      //-> where(iseven)
      //-> reverse()
      //-> skip(2)
      //-> take(4)
      //-> distinct()
      //-> select(tostr);

    //FOR(linq.Count)
        //puts((char*)linq.selbuf[i]);
    
    //getchar();
    return 0;
}

/*
[done] - check out OMAR's #ifdef idea
- turn exceptions off
- access stuff from the C compiler (debugging symbols etc)
- fix prens generation indentation
[done] - static array with count
[done] - dynamic array (list)
[done] - dynamic dictionary/hashtable (via linked list)
[done] - macros (loops, linear search, allocators, etc)
[done] - same syntax for objects in stack/heap (via ->)
[done] - 'using'-like syntax
[done] - LINQ-like syntax
- checkout glib source code
- code generation/meta-programming
- serialization
- better printf/logger
- custom memory allocators
- research adding C type information
- error callbacks (ERROR(CODE, MESSAGE))

meta/introspection/reflection/AST stuff:
[done] - call function by Name
[done] - allocate object by type Name
[done] - get type names, member names
[done] - set/get member Value on an object by Name
[done] - obj-type-id bindings
- forward declarations
[done] - external generator exe
[done] - clean use
- address field assignments
- attributes
- generate types
- extend to serialization

*/

// There's an alternative way to start/end the counter instead of
// using constructor/destructor pair, and you don't have to store
// the data in the struct, maybe not as convenient but it's worth
// mentioning, I use it quite often when I have to 'begin X' --
// write code -- 'end X'. It's basically a hacked/tweaked version
// of C#'s 'using' statement I think you'll find it interesting,
// check out this small snippet if you please:
// http://pastebin.com/TeLgg05A

// What other useful preproc Values we could use other than
// __FILE__, __FUNCTION__ and __LINE__? (Note: you could
// pronounce my Name as "Elzen" for short)

// I'm not sure I fully understand Unity builds. So Unity build
// is when we compile everything to a single file? I don't think
// that's the case because we do have multiple source files in
// HMH...

// Why do you have both u32 and uint32 typedefs? what's the
// difference?

// If exceptions are bad, then what is a good way to handle
// errors? just return error codes? a global error Value like in
// errno or GetLastError()? what do you think of the idea of
// having a central error handling function that we call and pass
// it the error id and it acts accordingly (switch statement
// maybe)?

// ---

//[answered]
// You mentioned that we could use fwrite to write the memory
// block and we dno't need serialization. However; 0- Would
// fwrite/fread have endiannes issues on different platforms? 1-
// How would we handle versioning? Let's say we saved a struct,
// then removed or added field into it, how would we safely read
// it back? 2- What if the struct had pointers? say a char*, then
// fwrite would write the pointer Value not what it is pointing
// to...

//[answered]
// Is a C compiler faster in compiling C code than a C++ compiler compiling C-like C++?

//[answered]
//Is there a standard/portable way in C to a get a FILE* to a
//block/region in memory?  In Linux there's fmemopen but I can't
//seem to find an equivalent in Windows.  Internet seems to
//suggest MapViewOfFile but I don't think that is it, I don't
//want a memory mapped file, just FILE* to a memory region. Does
//such a thing exist or am I looking at this the wrong way?

//[answered]
// Why is the preprocessor able to convert a parameter to a
// string via the # symbol, but not the other way around? i.e.
// string stripping, taking the inner Value of the string and
// pasting it. My guess is that it's because this would not make
// it possible for the macro to be determined at compile time
// since we can pass it arbitrary strings?

// You type really fast! My current max is 115 WPM, and average
// in the 100s. What is your WPM speed? (Word Per Minute). I'm
// really curious, if you don't know you could take take a
// 1-minute type test here: http://10fastfingers.com/  my guess
// is that at the least you could easily be at the average of
// 130+ WPM

//[answered]
// yesterday I asked about preprocessor constants other than
// __FILE__, __LINE__ etc I did some searching and found out
// about __TIME__ and __DATE__. Maybe it's useful for us to
// include a date-time stamp in some of our logs?

//[answered]
// Metaprogramming question: Let's say I want to have a generic
// dynamic list (resizable array) in my game library, void
// pointers and macros are two ways of approaching this, another
// is generating the code for the unique list types I want, is
// this what you mean when you say "I write C programs that spits
// out C programs" - If so, can we do this generation at
// compile-time (like we can do in jai :p) or are we forced to
// write and run an executable that does the generation for us?

// You seem to be consistent in your naming convention, TitleCase
// for variables/functions and snake_case for types.  Have you
// ever had this issue where you couldn't settle on a certain
// style? or have you always been consistent from day 1? I've
// programed in almost all possible naming conventions/styles.
// snake_case, camelCase, TitleCase, smallcase and hungarian
// notation. I like smallcase (DOOM style) and snake_case for
// variables/functions because they're easier to type and I make
// less typos in them, Snake_Case/TitleCase for types. The
// problem is I can't seem to settle on a certain style cause I
// like both, and so sometimes I decide to swtich from one to
// another wasting time renaming stuff...

//[answered]
// You mentioned yesterday that destructors are called when the
// scope of the object ends. I just wanted to note I recently read about 'exit' and it
// seems that if you use it to exit a function instead of
// 'return' then the destructors won't get called!

//[answered]
// I noticed you prefer to keep things on the stack and return by
// Value. 0- this helps with locality of reference and is more
// cache friendly am I right? 1- is the stack size something to
// be concerned about?  2- when returning by Value, is the time
// taken to copy the object and return it something to be worried
// about? maybe when the struct is large? In that case do you
// prefer to return a pointer to the object?

//[answered]
// Why is reflection slow in high-level languages? I finally got
// around to do some metaprogramming in C and it was awesome!  I
// wrote simple a reflection system that lets you call functions
// by Name, allocate types by Name/id, get/set global/instance
// fields by Name, you can query a type fields (Name/type), query
// function parameters etc And it was very fast! (will be sharing
// the code in hmh forums). Thank you for making me go back to C
// :)

//[answered]
// Do you play chess? If so maybe we can play together sometime! :D

//[answered]
// You mentioned that you use a known base address for your memory management,
// could you talk a bit more about that? does that mean I can now find things
// but just offsetting from that address? and does it mean that if I fwrite this
// whole block I'll essentially be fwriting the whole game?

//[answered]
// Do you keep track of struct padding when you add/remove
// fields?  or is it something you don't think about too much,
// so order of fields doesn't matter much?

//[answered]
// do you ever find use size_t or do you just use u32, u64 etc?

//[ignored]
// What do you think about impelmenting a SOLID abstract pure
// virutal generic IoC Container composite to encapsulate private
// singleton object members inherting from our base abstract
// factory method to build the polymorphic iterator adapter proxy
// in order propagate the chain of responsibility via the visitor
// and/or mediator pattern? Kappa

//[answered]
//Q: why are compilers not very informative in their error messages?
//why can't it be straight and just tell me about the missing SemicolinToken?

//[answered]
//Q: For metaprogramming, what do you think is a good way to
//parse C code to collection information about structs, fields,
//functions etc. Do you load the file and read it char by char?
//line by line? or use regex? do you do multiple passes over the
//file or a single pass is enough? do you look at .h or .cpp or
//both? I'd really appreciate some pointers on this one! Thanks.

//[answered]
//Q: What do you think is an effective way to get better at assembly?
//Go over a book/tutorial? read the code disassembly in VS?  Mike
//Acton can look at a piece of code for a couple of seconds and
//estimate how much cycles it takes and what the assembly code for
//it is... I'd like to reach that level one day :D

//[answered]
//Q: How do you handle programmer burnout/depression? Those days when
//you're just not in the mood to do anything, not productive,
//you're demotivated, you can barely do any coding, you'd rather
//do 'anything' else other than what you should be doing.

//[answered]
//Q: Why don't compilers expose API that gives us information about
//our code?  (access to the AST etc). Because when we write a
//top-down parser we're essentially writing something that the
//compiler already has.

//[answered, day 183]
//When do you think it is a good idea to use somebody's else
//library/system instead of implementing our own? (e.g. do you
//use anything from the standard C library?)

//[answered]
//Q: what would be the most important things you'd look at when hiring a programmer?
//what type of questions would you ask? does he have to have like 20+ years of experience?

//[answered]
//Q: Does it ever happen to you when you sleep on a problem and then
//when you wake up the solution would just click in your head?

//[answered]
//Q: are you doing any new Jeff & Casey Show stuff?

//[answered]
//Q: what do you think of the theory that says we already know
//everything, all the knowledge there is but we just don't
//remember it. so when we study etc we start remembering
//things...

//[answered]
//Q: do you use the same memory model as in hmh when writing
//utilities or meta programs?  or do you just malloc and it
//wouldn't matter because the thing runs once, does its thing and
//closes?

//[answered]
//Q: thank you very much for mentioning cmirror. it addresses a
//lot of my issues directly and is much more human readable :)
//What was the GetToken written for? for config files? doesn't
//seem like for C code

//[answered]
//Q: does it ever happen to you when you solve a problem in a
//certain way and then later on you find a much more simpler
//approach and you go 'oh... why haven't I thought of that?' - (I
//used to suffer from this a lot back when I wrote 'oop' code.
//it just makes trivial things so ridiculously complex...)

//[answered]
//Q:why is the standard libraries in most languages are not very
//good that we end up writing our own libraries? why are they
//even called 'standard' if they're bad?

//[answered]
//Q: a couple streams ago you did a pretty cool water bubble mouth
//sound effect, can you teach us how to do it? :D

//[answered]
//Q: I noticed you had the intel guide/intrinsics page in one of the
//prestreams, does it matter or make a difference if users have AMD
//processors?

//[answered]
//Q: is there a reason why we have many different starting points
//for coorindate systems (center vs top-left vs top-right
//corners: windows vs direct3d vs opengl) - wouldn't our lives be
//easier if there was a standard for them?

//[answered]
//Q: does the 0.5 pixel error matter a lot? I mean it seemed a
//pretty easy thing to overlook, would it make some calculations
//erroneous?

//[answered]
//Q: how much 'valuable' do you think C++ operator overloading
//actually is?  I'm thinking of fully moving to just pure C. do you
//think it would be inconvenient/a hassle to program without them?
//(vector operations etc)

//[answered]
//Q: is writing "typedef struct X { } X;" vs "struct X { };"
//purely a C vs C++ syntax thing? I noticed you do both in
//different areas of the code. any reason to use on over the
//other?

//[answered]
//Q: what do we have both bool32 and int32?

//[answered]
//Q: can you think of a pure C way to implement our time blocks
//instead of using ctor/dstor? manually inserting begin/end calls?

//[answered: day 189]
//Q: in yesterday's prestream you mentioned that you 'bundle' the count
//with your array, do you mean like how Sean does it in his
//stretchy_buffer.h? (allocates two integers for the count and
//capacity in the beginning of the array memory (ptr) and return
//to the user ptr + 2)

//[answered]
//Q: do you have any thoughts on the C11 '_Generic' feature?
//example usage: http://stackoverflow.com/questions/9804371/syntax-and-sample-usage-of-generic-in-c11

//[answered: cmirror]
//Q: you recommended me to look at TinyC to see how it does its
//parsing, but it was a bit complex I couldn't really tell where
//to look in the source code. do you have any other
//recommendations i could lookup? do I have to go all
//computer-science on this and write the language grammar,
//parsing table, etc or is it simpler than that? I've approached
//it many times but it's always hacky so I thought I must be
//missing a core idea

//[answered]
//Q: what you think of C++'s type-inference 'auto' feature? does
//it affect compilation speed? do you think it's useful to use
//when the type name is long and tedious to type? i.e. auto x =
//GetMySuperLongThing(); instead of SuperLongThing x =
//GetMySuperLongThing(); (of course I wouldn't use it when it
//obfuscates the return type of the expression, only when the
//return type is obvious to the reader)

//[answered]
//Q: could you please elaborate a bit on cmirror's string_table
//struct? it has a StoreBase and StoreCurrent. do you allocate a
//single giant string in StoreCurrent and use it throughout the
//program instead of small string chunks all over the place? (I
//also noticed there's no usage of StoreBase)

//[answered]
//Q: How do you handle invalid tokens when parsing user source
//files?  do you print out an error message and continue parsing or
//abort?

// Q: camera on hand/keyboard?

//[answered]
//Q: there are magenta spikes in the debug graph that happens so
//fast we don't have time to click on them, how do we plan to
//catch those? having the ability to step-over frame-by-frame
//would be useful me thinks. another idea is to have the debug text left
//off at the last rect position we hovered at

//Q: twitch reports ~2.75 million total views on handmade hero.
//is that number accuate?

//Q: Just found out today that function() in C means it could
//take arbitrary number of parameters while function(void) means
//no parameters. But in C++ function() and function(void) are the
//same (no params)

    //Jon
//How do you normally implement your
//meta-programming/introspection systems in C/C++?  do you
//#define a meta macro that does nothing and write a parser to
//collect the data and generate meta-initialization code?

    //Jon
//I was wondering how you handle pointers in your serialization
//systems, because it could be pointing to a single object or a
//dynamic array, do you add meta-annotations to help the
//serializer figure out the array count, etc?
