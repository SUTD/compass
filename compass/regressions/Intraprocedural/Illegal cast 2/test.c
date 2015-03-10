// Intraprocedural/Illegal cast



struct a
{
  char y[30];
  int x;
};

struct b
{
  short z;
};

void test(struct b* bb)
{
  //should report casting error
  struct a* aa = (struct a*) bb;
  bb->z = 77;
}