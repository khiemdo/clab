
struct Verticies;
struct Inventory;

meta() struct Player
{
    //u32 id = 0, weapon = 1;
    float health = 100.0f;
    float speed;
    Verticies *mesh, *model;
    Inventory *inventory;
};

meta()
int result;

meta()
void proc0() { puts("proc0"); }

meta()
int sum(int x, int y) { return x + y; }

meta()
void proc1(int *ptr0, float* *ptr1, double* ptr2) { }
