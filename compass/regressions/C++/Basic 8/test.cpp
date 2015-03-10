
int foo2()
{
  return 1;
}


void boo()
{
  int x = foo2();
  static_assert(x == 1);
}