void foo(int* a)
{
  int b;
  bar(a, &b);
}

void bar(int* a, int *b)
{
  *a = 2;
  *b = 3;
  static_assert(*a != *b);
}