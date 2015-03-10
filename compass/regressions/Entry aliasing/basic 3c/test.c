void foo(int* a, int* b)
{
  int t = *a;
  int z = *b;
  *a = 2;
  *b = 3;
   static_assert(*a != *b); //ok
}

void bar()
{
  int x;
  int y;
  foo(&x, &y);
}