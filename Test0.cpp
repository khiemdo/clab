
meta(serializable[custom=true])
struct TestStruct
{
    meta(helpbox[text="meta message...", comment=true],
         popup[from="getvalues", value=3.14f])
    string message;
};

