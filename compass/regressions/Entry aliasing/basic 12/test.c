void foo(int* a)
{
  bar(a, a);
 
}

void bar(int* a, int *b)
{
  *a = 2;
  *b = 3;
  static_assert(*a != *b); //should fail
}