

int foo(int* a, int* b)
{
  b[2] = 34;
  int x = a[3];
  return x;
}

void bar()
{
  int a[4];
  a[3] = 55;
  int r = foo(&a[0], &a[1]);
  static_assert(r == 55); //should fail
}