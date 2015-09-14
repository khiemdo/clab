
#include "common.h"
#include "meta.h"
#include "lab.h"

#include "Test0.cpp"
#include "Test1.cpp"

#include "meta.gen"
#include "collections.cpp"

#define LIST(T) LIST_##T
#define ARRAY(T) ARRAY_##T

void main()
{

    // get/set global fields dynamically
    int ten = 10;
    META_SET_GLOBAL("result", &ten);
    int back = META_GET_GLOBAL("result", int);

    // invoke function dynamically
    int five;
    META_CALL("sum", &five, 2, 3);

    // create objects dynamically
    Player* p = (Player*)META_ALLOC("Player");

    // set/get fields dynamically
    META_SET_MEMBER(p, "id", &five);
    int id = *(int*)META_GET_MEMBER(p, "id");

    // iterate over a type fields and view their name and type
    u32 pid = type_id(p); // or META_TYPE_ID("Player")
    mt_fields& player_fields = metatypes.fields[pid];
    FOR(player_fields.count)
        printf("%s Player.%s\n", player_fields.types[i], player_fields.names[i]);

    puts("");

    // iterate over global fields
    FOR(metafields.count)
        printf("%s %s\n", metafields.types[i], metafields.names[i]);

    puts("");

    getchar();

    //// iterate over functions and read their parameters
    //FOR(metafuncs.count)
    //{
        //printf("%s (", metafuncs.names[i]);
        
        //MT_Fields& params = metafuncs.params[i];
        //FOR(params.count)
            //printf("%d: %s %s ", i, params.types[i], params.names[i]);
        
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

    //FOR(linq.count)
        //puts((char*)linq.selbuf[i]);
    
    //getchar();
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
[done] - call function by name
[done] - allocate object by type name
[done] - get type names, member names
[done] - set/get member value on an object by name
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

// What other useful preproc values we could use other than
// __FILE__, __FUNCTION__ and __LINE__? (Note: you could
// pronounce my name as "Elzen" for short)

// I'm not sure I fully understand Unity builds. So Unity build
// is when we compile everything to a single file? I don't think
// that's the case because we do have multiple source files in
// HMH...

// Why do you have both u32 and uint32 typedefs? what's the
// difference?

// If exceptions are bad, then what is a good way to handle
// errors? just return error codes? a global error value like in
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
// fwrite would write the pointer value not what it is pointing
// to...

//[answered]
// Is a C compiler faster in compiling C code than a C++ compiler compiling C-like C++?

//[answered]
// Is there a standard/portable way in C to a get a FILE* to a
// block/region in memory?  In Linux there's fmemopen but I can't
// seem to find an equivalent in Windows.  Internet seems to
// suggest MapViewOfFile but I don't think that is it, I don't
// want a memory mapped file, just FILE* to a memory region. Does
// such a thing exist or am I looking at this the wrong way?

//[answered]
// Why is the preprocessor able to convert a parameter to a
// string via the # symbol, but not the other way around? i.e.
// string stripping, taking the inner value of the string and
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
// value. 0- this helps with locality of reference and is more
// cache friendly am I right? 1- is the stack size something to
// be concerned about?  2- when returning by value, is the time
// taken to copy the object and return it something to be worried
// about? maybe when the struct is large? In that case do you
// prefer to return a pointer to the object?

//[answered]
// Why is reflection slow in high-level languages? I finally got
// around to do some metaprogramming in C and it was awesome!  I
// wrote simple a reflection system that lets you call functions
// by name, allocate types by name/id, get/set global/instance
// fields by name, you can query a type fields (name/type), query
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
// virutal generic IoC container composite to encapsulate private
// singleton object members inherting from our base abstract
// factory method to build the polymorphic iterator adapter proxy
// in order propagate the chain of responsibility via the visitor
// and/or mediator pattern? Kappa

//[answered]
//Q: why are compilers not very informative in their error messages?
//why can't it be straight and just tell me about the missing semicolin?

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

//Q: are you doing any new Jeff & Casey Show stuff?

//Q: what do you think of the theory that says we already know everything, all the knowledge there is but we just don't remember it. so when we study etc we start remembering things...

//Q: do you use the same memory model as in hmh when writing utilities or meta programs?
//   or do you just malloc and it wouldn't matter because the thing runs once, does its thing
//   and closes?
