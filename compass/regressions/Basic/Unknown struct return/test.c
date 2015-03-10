
struct Y
{
  int c;
  int d;
};

struct X
{
  int a;
  struct Y* b;
};

struct X unknown();

void foo()
{
struct X my_y;
struct X my_a = my_y;
struct X my_x = unknown(); 
my_x.a = 1;
my_x.b->d = 2;
static_assert(my_x.b->d == 2);
}