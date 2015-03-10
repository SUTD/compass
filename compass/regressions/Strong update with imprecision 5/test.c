#include <stdlib.h>


void foo(int* a)
{
  if(rand()) *a = 1;
  else if(!rand()) *a = 2;
  else *a = 3;
}

void bar()
{
  int t = 0;
  foo(&t);
  static_assert(1<=t && t<=3);
}