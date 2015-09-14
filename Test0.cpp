
meta(serializable[custom=true])
struct TestStruct
{
    string message;
};

meta(helpbox[text = "meta message...", comment = true],
     popup[from = "getvalues"])
string global_popup;

meta(button[text="Select"])
void select_button()
{
}
