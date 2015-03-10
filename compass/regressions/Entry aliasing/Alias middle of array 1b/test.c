char foo(char* c, char* b)
{
  *b = 'f';
  char x = c[1];
  return x;
}

void bar()
{
  char a[4];
  a[1] = 'y';
  char r = foo(&a[0], &a[1]);
  static_assert(r == 'f'); //ok
}