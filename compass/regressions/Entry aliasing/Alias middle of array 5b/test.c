int foo(int* c, int* b)
{
  *b = 34;
  int x = c[1];
  return x;
}

void bar()
{
  int a[4];
  int b[4];
  a[1] = 55;
  int r = foo(&a[0], &b[1]);
  static_assert(r == 55); //ok
}