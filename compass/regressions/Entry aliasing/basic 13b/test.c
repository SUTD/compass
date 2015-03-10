





int foo(int* x, int* y)
{
  *x = 2;
  int t = *y;
  return t;
}

void bar()
{
  int a = 4;
  int r = foo(&a, &a);
  static_assert(r == 4); //should fail
 
}
