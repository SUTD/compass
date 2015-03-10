void foo(int x, int y)
{

  int* a= &x;
  int* b = &x;
  static_assert(a!=b);
}