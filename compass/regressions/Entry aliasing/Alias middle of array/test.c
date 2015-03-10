char foo(char* a, char* b)
{
  *b = 'f';
  char x = a[1];
  return x;
}

void bar()
{
  char a[4];
  a[1] = 'y';
  char r = foo(&a[0], &a[1]);
  static_assert(r == 'f'); //ok
}