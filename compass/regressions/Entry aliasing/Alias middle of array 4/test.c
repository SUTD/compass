int foo(int* a, int* b)
{
  *b = 34;
  int x = a[1];
  return x;
}

void bar()
{
  int a[4];
  a[1] = 55;
  int r = foo(&a[0], &a[1]);
  static_assert(r == 55); //should fail
}