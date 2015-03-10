void foo(int* a, int* b)
{
  int t = *a;
  int z = *b;
  *a = 2;
  *b = 3;
   static_assert(*a != *b); //should fail
}

void bar()
{
  int x;
  foo(&x, &x);
}