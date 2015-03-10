#include <stdlib.h>


void foo(int* a)
{
  if(rand()) *a = 1;
  else *a = 2;
}

void bar()
{
  int t = 0;
  foo(&t);
  static_assert(t!=0);
}