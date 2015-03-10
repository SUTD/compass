void foo(int x, int y)
{

  int* a= &x;
  int* b = &y;
  static_assert(a!=b);
}