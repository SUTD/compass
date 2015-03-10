
int foo()
{
  static int x = 0;
  x++;
  return x;
}

void test()
{
  int r1 = foo();
  static_assert(r1 == 1); //correct
  int r2 = foo();
  static_assert(r2 == 2); //correct
}