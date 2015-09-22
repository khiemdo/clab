#include "common.h"

/*
    current limitations:
    [line comments only at the momment] - meta() inside comments
    [done] - any * or & in function parameter or field must be accompanied with type and not the parameter/variable Name
    [done] - field assignments are not ignored yet and will confuse the parser
    [done] - META_INIT hack
    [done] - generation-time check for valid notes/Keys names
    - more information: is_pointer, is_array, is_pod, etc
    - include = in function default parameters
*/

#define MAX_FUN_ARGS 32
#define MAX_STRUCT_FIELDS 128

#define W(out, line, ...) sprintf((out) + strlen((out)), line, __VA_ARGS__)
#define WL(out, line, ...) W(out, line"\n", __VA_ARGS__)
#define WI(out, line, ...) W(out, "\t"line, __VA_ARGS__)
#define WIL(out, line, ...) WI(out, line"\n", __VA_ARGS__)

#define KB(x) (x * 1024LL)
#define MB(x) (KB(x) * 1024LL)

#define Error(Message, ...) Assert(0, Message, __VA_ARGS__)

//TODO minus sign for negative values? Check for default negative values make sure it works
enum token_type
{
    SemicolinToken,
    LParensToken,
    RParensToken,
    LBracketToken,
    RBracketToken,
    LBraceToken,
    RBraceToken,
    EqualsToken,
    CommaToken,
    StarToken,
    AmpersandToken,
    MetaToken,
    StructToken,
    EOFToken,
    IdentifierToken,
    StringToken,
    BooleanToken,
    NumberToken,
    UnknownToken
};

struct token
{
    token_type Type;
    string Content;
};

struct string_storage
{
    //string MetaTypes;
    //string MetaFuncs;
    //string TypeAllocs;
    //string FunCalls;
    //string TypeIds;
    //string GlobalGetSet;
    //string InstanceGetSet;

    char MetaTypes[MB(16)];
    char MetaFuncs[MB(8)];
    char TypeAllocs[MB(8)];
    char FunCalls[MB(16)];
    char TypeIds[MB(8)];
    char GlobalGetSet[MB(8)];
    char InstanceGetSet[MB(16)];
} Out = {};

struct note_param
{
    string Keys[32];
    string Values[32];
    u8     Count;

    void Add(string Key, string Value)
    {
        Assert(Count < 32);
        Keys[Count] = Key;
        Values[Count++] = Value;
    }
};

struct notes
{
    string      Names[32];
    note_param  Params[32];
    u32         Count;
};

internal bool HasKnownNotes = false;

struct
{
    u32      Count;
    string*  Names;
    string** Keys;
} KnownNotes = {};

struct field_declaration
{
    StaticArray(string, Names, 32);

    // When need an array of types in cases where:
    // int **x, *y, &z;
    StaticArray(string, Types, 32);

    notes Notes;
};

struct function_declaration
{
    string Type;
    string Name;
    notes  Notes;
    StaticArray(string, ParamNames, 32);
    StaticArray(string, ParamTypes, 32);
};

struct struct_declaration
{
    string Name;
    notes  Notes;
    DynamicArray(field_declaration, Fields);

    u32 GetNumFields()
    {
        u32 Result = 0;
        FOREACH(Fields)
            Result += it.TypesCount;
        return Result;
    }
};

internal inline
bool IsNumeric(char Input)
{
    return Input >= '0' && Input <= '9';
}

internal inline
bool IsAlpha(char Input)
{
    return (Input >= 'a' && Input <= 'z') ||
           (Input >= 'A' && Input <= 'Z');
}

internal inline
bool IsEOL(char Input)
{
    return Input == '\r' || Input == '\n';
}

internal inline
bool IsWhitespace(char Input)
{
    return Input == ' ' || Input == '\t';
}

internal void
SkipToNextLine(string& Input)
{
    while(*Input && !IsEOL(*Input))
        Input++;

    if ((Input[0] == '\r' && Input[1] == '\n') ||
        (Input[0] == '\n' && Input[1] == '\r'))
        Input += 2;
    else if(IsEOL(*Input))
        Input++;
}

internal string
ParseString(string& Input)
{
    Assert(*Input == '"');

    string Result = SALLOC(512);
    int i = 0;
    Result[i++] = *Input++;
    while(*Input && *Input != '"')
        Result[i++] = *Input++;
    Result[i] = '"';
    Result[i + 1] = '\0';

    if (*Input == '"')
        Input++;
    else
        printf("Invalid end of string literal %c\n", *Input);

    return Result;
}

internal string
ParseIdentifier(string& Input)
{
    Assert(IsAlpha(*Input));

    string Result = SALLOC(128);
    int i = 0;
    while(IsAlpha(*Input) || IsNumeric(*Input))
        Result[i++] = *Input++;
    Result[i] = '\0';

    return Result;
}

internal void
ParseNumber(string& Input, string& Result)
{
    Assert(IsNumeric(*Input));

    bool Decimal = false;
    bool Parsing = true;
    int i = 0;
    while(Parsing)
    {
        if (IsNumeric(*Input))
        {
            Result[i++] = *Input++;
        }
        else
        {
            if (*Input == '.')
            {
                if (Decimal)
                    Error("Invalid numeric value");
                Decimal = true;
                Result[i++] = *Input++;
            }
            else if (*Input == 'f' && Decimal)
            {
                Result[i++] = *Input++;
                Parsing = false;
            }
            else
            {
                Parsing = false;
            }
        }
    }
    Result[i] = '\0';
}

internal token
GetToken(string& Input)
{
    while(*Input)
    {
        if((Input[0] == '/') && (Input[1] == '/'))
        {
            Input += 2;
            SkipToNextLine(Input);
        }
        else if(IsWhitespace(*Input))
        {
            Input++;
        }
        else if(IsEOL(*Input))
        {
            SkipToNextLine(Input);
        }
        else
        {
            break;
        }
    }

    token Token;
    switch(*Input)
    {

        case '=':
            Token.Type = EqualsToken;
            Token.Content = "=";
            Input++;
            break;

        case '*':
            Token.Type = StarToken;
            Token.Content = "*";
            Input++;
            break;

        case '&':
            Token.Type = AmpersandToken;
            Token.Content = "&";
            Input++;
            break;

        case ';':
            Token.Type = SemicolinToken;
            Token.Content = ";";
            Input++;
            break;

        case ',':
            Token.Type = CommaToken;
            Token.Content = ",";
            Input++;
            break;

        case '(':
            Token.Type = LParensToken;
            Token.Content = "(";
            Input++;
            break;

        case ')':
            Token.Type = RParensToken;
            Token.Content = ")";
            Input++;
            break;

        case '{':
            Token.Type = LBraceToken;
            Token.Content = "{";
            Input++;
            break;

        case '}':
            Token.Type = RBraceToken;
            Token.Content = "}";
            Input++;
            break;

        case '[':
            Token.Type = LBracketToken;
            Token.Content = "[";
            Input++;
            break;

        case ']':
            Token.Type = RBracketToken;
            Token.Content = "]";
            Input++;
            break;

        case '\0':
            Token.Type = EOFToken;
            Token.Content = "";
            break;

        case '"':
            Token.Type = StringToken;
            Token.Content = ParseString(Input);
            break;

        default:
        {
            if (IsAlpha(*Input))
            {
                Token.Content = ParseIdentifier(Input);
                if (streql(Token.Content, "meta"))
                    Token.Type = MetaToken;
                else if (streql(Token.Content, "struct"))
                    Token.Type = StructToken;
                else if (streql(Token.Content, "true") ||
                         streql(Token.Content, "false"))
                    Token.Type = BooleanToken;
                else
                    Token.Type = IdentifierToken;
            }
            else if (IsNumeric(*Input))
            {
                Token.Type = NumberToken;
                string Result = SALLOC(128);
                ParseNumber(Input, Result);
                Token.Content = Result;
            }
            else
            {
                Token.Type = UnknownToken;
                Token.Content = Input;
                Input++;
            }
        }
    }

    return Token;
}

internal bool
GetTokenOfType(string& Input, token_type WantedType, token& Result)
{
    Result = GetToken(Input);
    return Result.Type == WantedType;
}

internal void
ReadNotes(notes& Notes, string& Input)
{
    // e.g. (helpbox[text="hello", type=comment], popup[from="GetValues"])

    FOR(Notes.Count)
        Notes.Params[i].Count = 0;
    Notes.Count = 0;

    bool DoneParsing = false;
    while(!DoneParsing)
    {
        var Token = GetToken(Input);
        string NoteName;
        switch(Token.Type)
        {
            case RParensToken:
            {
                DoneParsing = true;
            } break;

            case CommaToken:
            {
                continue;
            } break;

            case IdentifierToken:
            {
                NoteName = Token.Content;
                Notes.Names[Notes.Count] = NoteName;
                if (HasKnownNotes)
                {
                    bool Found = false;
                    FOR(KnownNotes.Count)
                    {
                        if (streql(KnownNotes.Names[i], NoteName))
                        {
                            Found = true;
                            break;
                        }
                    }
                    if (!Found)
                        Error("Couldn't find note `%s` in the specified notes file\n", NoteName);
                }
            } break;

            case LBracketToken:
            {
                bool DoneParsingParams = false;
                bool ParsedNoteKey = false;
                string NoteKey = null;
                string NoteValue = null;

                while(!DoneParsingParams)
                {
                    Token = GetToken(Input);

                    switch(Token.Type)
                    {
                        default: Error("Unexpected token %s while parsing note %s",
                                         Token.Content, NoteName);

                        case LBracketToken:
                        {
                            continue;
                        } break;

                        case EqualsToken:
                        {
                            ParsedNoteKey = true;
                        } break;

                        case CommaToken:
                        {
                            ParsedNoteKey = false;
                        } break;

                        case RBracketToken:
                        {
                            DoneParsingParams = true;
                            Notes.Count++;
                        } break;

                        case EOFToken:
                        {
                            Error("Reached EOF while parsing note %s", NoteName);
                        } break;

                        case IdentifierToken:
                        {
                            if (!ParsedNoteKey)
                            {
                                NoteKey = Token.Content;

                                if (HasKnownNotes)
                                {
                                    bool Found = false;
                                    for(int i = 0; i < KnownNotes.Count; i++)
                                    {
                                        for(int j = 0; j < 32; j++)
                                        {
                                            string CurrentKey = KnownNotes.Keys[i][j];
                                            if (CurrentKey == null || !*CurrentKey)
                                                break;

                                            if (streql(CurrentKey, NoteKey))
                                            {
                                                Found = true;
                                                break;
                                            }
                                        }
                                        if (Found) break;
                                    }
                                    if (!Found) Error("Couldn't find key `%s` in note `%s` in the specified notes file\n",
                                                NoteKey, NoteName);
                                }
                            }
                            else
                            {
                                NoteValue = Token.Content;

                                if (!NoteKey || !NoteValue)
                                    Error("Invalid note key/value after parsing comma token in note %s", NoteName);
                                Notes.Params[Notes.Count].Add(NoteKey, NoteValue);
                            }
                        } break;

                        case StringToken:
                        case NumberToken:
                        case BooleanToken:
                        {
                            if (!ParsedNoteKey)
                                Error("Unexpected string, number or boolean token '%s' before parsing note key in note '%s'",
                                        Token.Content, NoteName);

                            NoteValue = Token.Content;

                            if (!NoteKey || !NoteValue)
                                Error("Invalid note key/value after parsing comma token in note %s", NoteName);
                            Notes.Params[Notes.Count].Add(NoteKey, NoteValue);
                        } break;

                    }
                }
            } break;
        }
    }
}

internal void
WriteNotes(notes Notes, string Dest, string Owner, u32 OwnerIndex)
{
    WIL(Dest, "%s.Notes[%d].Count = %d;",
            Owner, OwnerIndex, Notes.Count);

    FOR(Notes.Count)
    {
        WIL(Dest, "%s.Notes[%d].Names[%d] = \"%s\";",
                Owner, OwnerIndex, i, Notes.Names[i]);

        printf("Writing note %s\n", Notes.Names[i]);

        note_param& p = Notes.Params[i];
        FORk(p.Count)
        {
            WIL(Dest, "%s.Notes[%d].Params[%d].Add(\"%s\", %s);",
                    Owner, OwnerIndex, i, p.Keys[k], p.Values[k]);

            printf("Note parameter %s\n", p.Keys[k]);
        }
    }
}

internal void
WriteField(string_storage& Out, field_declaration& Field, string StructName, u32 NumTypes)
{
    FOR(ArrayCount(Field.Types))
    {
        string& FieldName = Field.Names[i];
        string& FieldType = Field.Types[i];

        // Write field type and name
        WIL(Out.MetaTypes, "MetaTypes.Fields[%d].Types[%d] = \"%s\";",
                NumTypes, i, FieldType);
        WIL(Out.MetaTypes, "MetaTypes.Fields[%d].Names[%d] = \"%s\";",
                NumTypes, i, FieldName);

        // Is this a global field?
        if (StructName == null)
        {
            // Write global field getter/setter functions
            WL(Out.GlobalGetSet,
                    "inline void* get_%s(void* Target) { return &%s; }",
                    FieldName, FieldName);

            if (strlast(FieldType) == '*')
                WL(Out.GlobalGetSet,
                        "inline void set_%s(void* Target, void* Value) { %s = (%s)Value; }",
                        FieldName, FieldName, FieldType);
            else WL(Out.GlobalGetSet,
                    "inline void set_%s(void* Target, void* Value) { %s = *(%s*)Value; }",
                    FieldName, FieldName, FieldType);

            // Write global field getter/setter bindings
            WIL(Out.MetaTypes, "MetaTypes.Fields[0].Getters[%d] = get_%s;",
                    i, FieldName);
            WIL(Out.MetaTypes, "MetaTypes.Fields[0].Setters[%d] = set_%s;",
                    i, FieldName);

        }
        else
        {
            // Write instance field get/set functions
            WL(Out.InstanceGetSet,
                    "inline void* get_%s_%s(void* Target) { return &((%s*)Target)->%s; }",
                    StructName, FieldName, StructName, FieldName);

            if (strlast(FieldType) == '*')
                WL(Out.InstanceGetSet,
                        "inline void set_%s_%s(void* Target, void* Value) { ((%s*)Target)->%s = (%s)Value; }",
                        StructName, FieldName, StructName, FieldName, FieldType);
            else WL(Out.InstanceGetSet,
                    "inline void set_%s_%s(void* Target, void* Value) { ((%s*)Target)->%s = *(%s*)Value; }",
                    StructName, FieldName, StructName, FieldName, FieldType);

            // Write instance field get/set bindings
            WIL(Out.MetaTypes, "MetaTypes.Fields[%d].Getters[%d] = get_%s_%s;",
                    NumTypes, i, StructName, FieldName);
            WIL(Out.MetaTypes, "MetaTypes.Fields[%d].Setters[%d] = set_%s_%s;",
                    NumTypes, i, StructName, FieldName);
        }

        // Write field notes (if any)
        if (Field.Notes.Count)
        {
            string Owner = SALLOC(32);
            sprintf(Owner, "MetaTypes.Fields[%d]", NumTypes);
            WriteNotes(Field.Notes, Out.MetaTypes, Owner, i);
        }
    }
}

// void function(int x, int* y, int **z, int * w);
internal function_declaration
ParseFunction(string& Input, string FunctionType)
{
    function_declaration Result = {};
    Result.Type = FunctionType;
    char StarsAndAmpersands[4] = {};
    bool DoneParsing = false;

    // parse function type and name first
    while(!DoneParsing)
    {
        token Token = GetToken(Input);
        switch(Token.Type)
        {
            case EOFToken:
            {
                Error("Reached EOF while parsing function header");
            } break; 

            case StarToken:
            case AmpersandToken:
            {
                strcat(StarsAndAmpersands, Token.Content);
            } break;

            case IdentifierToken:
            {
                Result.Name = Token.Content;
            } break;

            case LParensToken:
            {
                if (*StarsAndAmpersands)
                    strcat(Result.Type, StarsAndAmpersands);
                DoneParsing = true;
            } break;

            default: Error("Unexpected token %s", Token.Content);
        }
    }

    bool ParsedParamType = false;
    DoneParsing = false;

    // parse parameters
    while(!DoneParsing)
    {
        token Token = GetToken(Input);
        switch(Token.Type)
        {

            case EOFToken:
            {
                Assert(DoneParsing, "EOF reached while parsing function parameters '%s'", Result.Name);
                DoneParsing = true;
            } break; 

            case StarToken:
            case AmpersandToken:
            {
                strcat(StarsAndAmpersands, Token.Content);
            } break;

            case IdentifierToken:
            {
                if (!ParsedParamType)
                {
                    ArrayAdd(Result.ParamTypes, Token.Content);
                    ParsedParamType = true;
                }
                else
                {
                    ArrayAdd(Result.ParamNames, Token.Content);
                    if (*StarsAndAmpersands)
                    {
                        // sanity checks
                        Assert(ArrayCount(Result.ParamTypes) > 0);
                        Assert(ArrayLast(Result.ParamTypes));

                        strcat(ArrayLast(Result.ParamTypes), StarsAndAmpersands);
                        *StarsAndAmpersands = '\0';
                    }
                    ParsedParamType = false;
                }
            } break;

            case CommaToken:
            {
                continue;
            } break;

            case RParensToken:
            {
                DoneParsing = true;
            } break;

            case EqualsToken:
            {
                GetToken(Input); // eat assignment value
            } break;

            default: Error("Unexpected token %s", Token.Content);
        }
    }

    return Result;
}

// int x;
// int x = 10;
// int x, y;
// int x = 20, y;
// int x, y = 30;
// int x = 10, y = 30;
// int x = 10, y = 30, z;
// int x = 10, y = 30, z = 40;
//TODO support arrays [] and StaticArray macro
internal field_declaration
ParseField(string& Input, string FieldType)
{
    field_declaration Result = {};

    bool DoneParsing = false;
    bool ParsedComma = true;
    char StarsAndAmpersands[4] = {};

    while(!DoneParsing)
    {
        token Token = GetToken(Input);
        switch(Token.Type)
        {

            case EOFToken:
            {
                Assert(DoneParsing, "Reached EOF while processing field declaration");
                DoneParsing = true;
            } break; 

            case StarToken:
            case AmpersandToken:
            {
                strcat(StarsAndAmpersands, Token.Content);
            } break;

            case IdentifierToken:
            {
                if (!ParsedComma)
                    Error("Missing comma after field name %s", ArrayLast(Result.Names));

                ParsedComma = false;

                ArrayAdd(Result.Names, Token.Content);
                ArrayAdd(Result.Types, strdup(FieldType));
                if (*StarsAndAmpersands)
                {
                    strcat(ArrayLast(Result.Types), StarsAndAmpersands);
                    *StarsAndAmpersands = '\0';
                }
            } break;

            case CommaToken:
            {
                ParsedComma = true;
            } break;

            case SemicolinToken:
            {
                DoneParsing = true;
            } break;

            case EqualsToken:
            {
                GetToken(Input); // eat assignment value
            } break;

            default: Error("Unexpected token %s", Token.Content);
        }
    }

    return Result;
}

internal struct_declaration
ParseStruct(string& Input)
{
    token Token;
    if (!GetTokenOfType(Input, IdentifierToken, Token))
        Error("Unexpected identifier '%s' after struct", Token.Content);

    struct_declaration Result = {};
    Result.Name = Token.Content;
    Result.Fields = CALLOC(field_declaration, MAX_STRUCT_FIELDS);

    if (!GetTokenOfType(Input, LBraceToken, Token))
        Error("Unexpected token. Expected '{' after struct '%s'", Result.Name);

    while(true)
    {
        string LookAhead = Input;
        notes FieldNotes = {};
        if (GetTokenOfType(LookAhead, MetaToken, Token))
        {
            if (!GetTokenOfType(LookAhead, LParensToken, Token))
                Error("Expected '(' after meta annotation for field in struct %s", Result.Name);

            Input += (LookAhead - Input);
         
            if (!GetTokenOfType(LookAhead, RParensToken, Token))
                ReadNotes(FieldNotes, Input);
        }

        if (!GetTokenOfType(Input, IdentifierToken, Token))
            Error("Unexpected token '%s' in struct '%s'. Expected an identifier (field type)",
                    Token.Content, Result.Name);

        var FieldType = Token.Content;
        var Field = ParseField(Input, FieldType);
        Field.Notes = FieldNotes;
        ArrayAdd(Result.Fields, Field);

        //TODO this doesn't support struct instances
        // e.g. struct  X { } x; problem is if we keep eating
        // till we reach a ';' then we're assuming that the '}'
        // we just found isn't the end of a function inside the struct
        LookAhead = Input;
        if (GetToken(LookAhead).Type == RBraceToken &&
            GetToken(LookAhead).Type == SemicolinToken)
        {
            Input += (LookAhead - Input);
            break;
        }
    }

    return Result;
}

// ( ) [ ] = ; meta , /**/ // * & "..."
// - variables/fields: found in global scope or inside structs
// - variable declarations end with a ';'
// - variable declarations could be accompanied with an assignement '=' which should be ignored
// - a meta() on mutiple declared variables gets applied on all of them
// - declaration follow the form:
// 0- Type Name;
// 1- Type Name = Value;
// 2- Type Name0 = Value, Name1;
// 3- Type Name0 = value0, Name1 = value1, etc;
// (separated by ',' assigned by '=' and ends with ';')
// - The meta annotation should come before the type Name
// - The pointer/addressof symbol should be coupled with the type and not the field Name
// i.e. int *x is basically int* x (type: int*, Name: x)
// What we care about reading when it comes to fields:
// 0- type 1- Name 2- meta notes (optionally the initial assigned Value via the '=')
// - functions: similar to variables, but we only care about the global ones
// - form: ReturnType Name(P0Type P0Name, P1Type P1Name = default_value, etc)
// - We care about the notes, return type, Name, parameter types and names,
//   we don't care about the default Values on parameters.
// - Type declarations: they start with the word 'struct' in global scope
// - We don't care about typedefs for now, just 'struct'
// - We care about their Name, fields and notes
// - When meta() is applied on a struct, all its fields are introspectable
// - Every type is given an id (starts from 1, 0 is reserved for globals)
// - Types could be allocated by Name at runtime
// - Should be able to parse .h and .c/.cpp files (the system keeps track of what has it added so far to avoid adding duplicate definitions or something)
// - What do we generate:
// - type allocators, type ids, global/member fields getters/setters and function callers
// - parsing functionalities:
//      get token
//      skip to end of line
//      read till a certain character

inline void
GenerateMeta(string OutPath, string NotesPath, u32 NumFiles, string* InFiles)
{
    FILE* MetaFile = fopen(OutPath, "w");
    if(MetaFile == null)
    {
        fprintf(stderr, "Error: Couldn't open output metafile '%s'", OutPath);
        return;
    }
    
    // See if we have a notes file specified
    string KnownNotesContent = ReadTextFile(NotesPath);
    if (KnownNotesContent)
    {
        string* NoteLines = null;
        KnownNotes.Count = strsplit(KnownNotesContent, ';', NoteLines);
        KnownNotes.Names = CALLOC(string, KnownNotes.Count);
        KnownNotes.Keys = CALLOC(string*, KnownNotes.Count);
        FOR(32) KnownNotes.Keys[i] = CALLOC(string, 32);
        HasKnownNotes = KnownNotes.Count > 0;
        FOR(KnownNotes.Count)
        {
            string Line = NoteLines[i];

            token Token;
            if (!GetTokenOfType(Line, IdentifierToken, Token))
                Error("Unexpected token in notes file %s. Expected an identifier (note name)", Token.Content);

            KnownNotes.Names[i] = Token.Content;

            if (!GetTokenOfType(Line, EqualsToken, Token))
                Error("Unexpected token in notes file %s. Expected '='", Token.Content);

            string* Keys = null;
            u32 NumKeys = strsplit(Line, ',', Keys);
            if (NumKeys > 32)
                Error("Too many keys in note %s. Max = %d", KnownNotes.Names[i], 32);

            FORj(NumKeys)
                KnownNotes.Keys[i][j] = Keys[j];
        }
    }

    u32 NumTypes = 1; // we start with at least one type (_GLOBAL_)
    u32 NumFuncs = 0;
    u32 NumGlobalFields = 0;

    //string_storage Out = {};

    FOR(NumFiles)
    {
        string FilePath = InFiles[i];
        string s = ReadTextFile(FilePath);
        if (!s)
        {
            fprintf(stderr, "Error: Couldn't read input file Content for parsing '%s'", FilePath);
            return;
        }

        notes MetaNotes = {};
        bool DoneParsing = false;
        bool ParsedMetaToken = false;

        while(!DoneParsing)
        {
            var Token = GetToken(s);
            switch(Token.Type)
            {
                case EOFToken:
                {
                    DoneParsing = true;
                } break;

                case MetaToken:
                {
                    if (!GetTokenOfType(s, LParensToken, Token))
                        Error("Expected '(' after meta token in file %s", FilePath);

                    // Read notes (if any)
                    string LookAhead = s;
                    if (GetTokenOfType(LookAhead, RParensToken, Token))
                        s += (LookAhead - s);
                    else 
                        ReadNotes(MetaNotes, s);

                    ParsedMetaToken = true;

                } break;

                case StructToken:
                {
                    if (!ParsedMetaToken)
                        continue;
                    ParsedMetaToken = false;

                    var Struct = ParseStruct(s);

                    // Write struct meta information
                    {
                        WIL(Out.MetaTypes, "MetaTypes.Names[%d] = \"%s\";",
                                NumTypes, Struct.Name);

                        FOREACH(Struct.Fields)
                            WriteField(Out, it, Struct.Name, NumTypes);

                        // Write struct number of fields
                        WIL(Out.MetaTypes, "MetaTypes.Fields[%d].Count = %d;",
                                NumTypes, Struct.GetNumFields());

                        // Write type allocator
                        WL(Out.TypeAllocs, "inline void* alloc_%s() { return CALLOC(%s, 1); }",
                                Struct.Name, Struct.Name);

                        // Bind alloctor function pointer
                        WIL(Out.MetaTypes, "MetaTypes.Allocs[%d] = alloc_%s;",
                                NumTypes, Struct.Name);

                        // Write type-id binding
                        WL(Out.TypeIds, "inline u32 TypeId(%s* ptr) { return %d; }",
                                Struct.Name, NumTypes);

                        if (MetaNotes.Count)
                            WriteNotes(MetaNotes, Out.MetaTypes, "MetaTypes", NumTypes);
                    }
                    NumTypes++;

                } break;

                case IdentifierToken:
                {
                    if (!ParsedMetaToken)
                        continue;
                    ParsedMetaToken = false;

                    string LookAhead = s;
                    while(*LookAhead && *LookAhead != '(' && *LookAhead != ',' && *LookAhead != ';')
                        LookAhead++;

                    if (*LookAhead != '(') // Global field?
                    {
                        var FieldType = Token.Content;
                        var Field = ParseField(s, FieldType);
                        WriteField(Out, Field, null, 0);
                        NumGlobalFields++;
                    }
                    else
                    {
                        var FunctionType = Token.Content;
                        var Function = ParseFunction(s, FunctionType);

                        // Write function meta information
                        {
                            // Write function type and Name
                            WIL(Out.MetaFuncs, "MetaFuncs.Types[%d] = \"%s\";",
                                    NumFuncs, Function.Type);
                            WIL(Out.MetaFuncs, "MetaFuncs.Names[%d] = \"%s\";",
                                    NumFuncs, Function.Name);

                            u32 NumParams = Function.ParamNamesCount;
                            if (NumParams == 0) // Has no params?
                            {
                                WIL(Out.MetaFuncs, "MetaFuncs.Params[%d].Count = 0;",
                                        NumFuncs);

                                // Write paramless funcall
                                WL(Out.FunCalls,
                                        "inline void call_%s(void* ret, ...) { %s(); }",
                                        Function.Name, Function.Name);

                                // Write paramless call bindings
                                WIL(Out.MetaFuncs, "MetaFuncs.Calls[%d] = call_%s;",
                                        NumFuncs, Function.Name);
                            }
                            else
                            {
                                W(Out.FunCalls, "inline void call_%s(void* ret, ...)\n"
                                        "{\n" "\tva_begin(ret);" "\n",
                                        Function.Name, Function.Name);

                                FOR(NumParams)
                                {
                                    string ParamType = Function.ParamTypes[i];
                                    string ParamName = Function.ParamNames[i];

                                    // Write param type and name
                                    WIL(Out.MetaFuncs, "MetaFuncs.Params[%d].Types[%d] = \"%s\";",
                                            NumFuncs, NumParams, ParamType);
                                    WIL(Out.MetaFuncs, "MetaFuncs.Params[%d].Names[%d] = \"%s\";",
                                            NumFuncs, NumParams, ParamName);

                                    // Fetch param from va_arg
                                    WIL(Out.FunCalls, "%s %s = va_arg(args, %s);",
                                            ParamType, ParamName, ParamType);
                                }

                                // Write funcall invocation
                                if (streql(Function.Type, "void"))
                                    W(Out.FunCalls, "\t" "%s(", Function.Name);
                                else
                                    W(Out.FunCalls, "\t" "*(%s*)ret = %s(",
                                            Function.Type, Function.Name);

                                FOR(NumParams-1)
                                    W(Out.FunCalls, "%s, ", Function.ParamNames[i]);
                                WL(Out.FunCalls, "%s);", Function.ParamNames[NumParams-1]);

                                // Write funcall tail 
                                WL(Out.FunCalls, "\t" "va_end(args);" "\n}");

                                // Write funcall pointer binding
                                WIL(Out.MetaFuncs, "MetaFuncs.Calls[%d] = call_%s;",
                                        NumFuncs, Function.Name);

                                // Write params count
                                WIL(Out.MetaFuncs, "MetaFuncs.Params[%d].Count = %d;",
                                        NumFuncs, NumParams);
                            }
                        }

                        if (MetaNotes.Count)
                            WriteNotes(MetaNotes, Out.MetaFuncs, "MetaFuncs", NumFuncs);

                        NumFuncs++;
                    }
                } break;

            }
        }
    }

    WIL(Out.MetaTypes, "MetaTypes.Names[0] = \"_GLOBAL_\";");
    WIL(Out.MetaTypes, "MetaTypes.Fields[0].Count = %d;", NumGlobalFields);
    WIL(Out.MetaTypes, "MetaTypes.Count = %d;\n",  NumTypes);
    WIL(Out.MetaFuncs, "MetaFuncs.Count = %d;\n",  NumFuncs);

    #define fwcomment(File, Comment, Block)\
        if (strlen(Block) > 0) fprintf(File, "\n// "Comment"\n%s", Block)

    fwcomment(MetaFile, "Type allocators", Out.TypeAllocs);
    fwcomment(MetaFile, "Function callers", Out.FunCalls);
    fwcomment(MetaFile, "Type-id bindings", Out.TypeIds);
    fwcomment(MetaFile, "Global variables [g|s]etters", Out.GlobalGetSet);
    fwcomment(MetaFile, "Instance Fields [g|s]etters", Out.InstanceGetSet);

    fprintf(MetaFile, "\n// Meta-data initialization\n");
    fprintf(MetaFile, "void meta_init()\n{\n" "%s%s" "}\n", Out.MetaTypes, Out.MetaFuncs);
    fprintf(MetaFile, "#undef META_INIT\n");
    fprintf(MetaFile, "#define META_INIT() meta_init()\n");

    fclose(MetaFile);

    #undef fwcomment
}

internal void
GenerateTypes(string OutPath, int NumArgs, string* Args)
{
    FILE* OutFile = fopen(OutPath, "wb");
    if (!OutFile)
    {
        fprintf(stderr, "failed to open template output %s", OutPath);
        return;
    }

    for(int x = 0; x < NumArgs; x += 2)
    {
        string InFile = Args[x];
        string Content = ReadTextFile(InFile);
        if (!Content)
        {
            fprintf(stderr, "failed to read file %s\n", InFile);
            continue;
        }

        string* SplitBySemicolin;
        string* SplitByComma;
        string* SplitByEquals;

        u32 NumSplitsSemicolin;
        u32 NumSplitsComma;
        u32 NumSplitsEquals;
        
        string Params = Args[x + 1]; // e.g. "X=string,Y=int,Z=float;X=u32,Y=float,Z=double";
        NumSplitsSemicolin = strsplit(Params, ';', SplitBySemicolin);

        FOR(NumSplitsSemicolin)
        {
            string s = Content;
            string SemicolinToken_split = SplitBySemicolin[i];
            NumSplitsComma = strsplit(SemicolinToken_split, ',', SplitByComma);

            Assert(NumSplitsComma <= 8, "NOT SUPPORTED! "
                                        "Why on earth would you have more than 8 type parameters? "
                                        "Consider re-evaluating your design.");
            char Keys[8];
            string Values[8];
            u32 kidx = 0;
            u32 vidx = 0;

            FOR(NumSplitsComma)
            {
                string CommaToken_split = SplitByComma[i];
                NumSplitsEquals = strsplit(CommaToken_split, '=', SplitByEquals);
                Assert(NumSplitsEquals == 2, "There should have been only one assignment operator. "
                                             "Multiple assignments are not supported.");
                Keys[kidx++] = SplitByEquals[0][0];
                Values[vidx++] = SplitByEquals[1];
            }

            while(*s)
            {
                bool Replaced = false;
                FOR(NumSplitsComma)
                {
                    if (s[0] == '$' && s[1] == Keys[i])
                    {
                        fprintf(OutFile, Values[i]);
                        s += 2; // assuming the user will always use $T, $K, $V, etc
                        Replaced = true;
                        break;
                    }
                }
                if (!Replaced)
                {
                    fprintf(OutFile, "%c", *s);
                    s++;
                }
            }
        }
    }
}

void main()
{
    string OutFile = "../meta.gen";
    string* InFiles = CALLOC(string, 32);
    u32 n = 0;
    InFiles[n++] = "../Test1.cpp";
    InFiles[n++] = "../Test0.cpp";
    string NotesPath = "../notes.meta";
    GenerateMeta(OutFile, NotesPath, n, InFiles);
    puts("Done"); getchar();
}

int dfsfain(int argc, string argv[])
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

        string OutFile = argv[2];
        string* InFiles = argv + 3;
        u32 NumArgs = (argc - 3) / 2;

        GenTypes(OutFile, NumArgs, InFiles);
    }
    else if (streql(mode, "-m"))
    {
        string OutFile = argv[2];
        string* InFiles = argv + 3;
        u32 NumArgs = argc - 3;

        gen_meta(OutFile, NumArgs, InFiles);
    }
    else if (streql(mode, "-s"))
    {
    }
    return 0;

#elif 0
    string* InFiles = CALLOC(string, 32);
    u32 n = 0;
    InFiles[n++] = "../list.template";
    InFiles[n++] = "T=string;T=int;T=float";
    InFiles[n++] = "../array.template";
    InFiles[n++] = "T=string;T=int;T=float";
    InFiles[n++] = "../test.template";
    InFiles[n++] = "X=string,Y=int,Z=float;X=u32,Y=float,Z=double";
    GenTypes("../collections.cpp", n, InFiles);

#elif 1
    string OutFile = "../meta.gen";
    string* InFiles = CALLOC(string, 32);
    u32 n = 0;
    InFiles[n++] = "../Test1.cpp";
    //InFiles[n++] = "../Test0.cpp";
    string NotesPath = "../notes.meta";
    GenerateMeta(OutFile, NotesPath, n, InFiles);
    puts("Done"); getchar();
#elif 0

    string Inputs[32];
    ArrayAdd(Inputs, "int* x;");
    ArrayAdd(Inputs, "int x = 10;");
    ArrayAdd(Inputs, "int &x, *y;");
    ArrayAdd(Inputs, "int x = 20, y;");
    ArrayAdd(Inputs, "int **x, y = 30;");
    ArrayAdd(Inputs, "int x = 10, y = 30, z;");

    FOREACH(Inputs)
    {
        var Field = ParseField(it);
        printf("Field:\n");
        FOR(Field.NamesCount)
        {
            printf("\tType: %s\n", Field.Types[i]);
            printf("\tName: %s\n", Field.Names[i]);
        }
    }

    getchar();
#elif 0
    StaticArray(string, Inputs, 32);
    InputsCount = 0;

    ArrayAdd(Inputs, "void function(int x, int* y, int *&z, int * w);");

    FOREACH(Inputs)
    {
        var Function = ParseFunction(it);
        printf("Function: Type=%s Name=%s\n", Function.Type, Function.Name);
        FOR(Function.ParamNamesCount)
        {
            printf("\t%s", Function.ParamTypes[i]);
            printf(" %s\n", Function.ParamNames[i]);
        }
    }

    getchar();
#elif 0
    StaticArray(string, Inputs, 32);
    InputsCount = 0;

    ArrayAdd(Inputs, "struct Test { int x; inventory* Inv; };");

    FOREACH(Inputs)
    {
        var Struct = ParseStruct(it);
        printf("Struct: Name=%s\n", Struct.Name);
        FOR(Struct.FieldsCount)
        {
            printf("\t%s",  Struct.Fields[i].Types[0]);
            printf(" %s\n", Struct.Fields[i].Names[0]);
        }
    }

    getchar();
#else
#endif

    return 0;
}
