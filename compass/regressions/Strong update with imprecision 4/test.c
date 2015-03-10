#include <stdlib.h>


void foo(int* a)
{
  if(rand()) *a = 1;
  else if(!rand()) *a = 2;
}

void bar()
{
  int t = 0;
  foo(&t);
  //should fail
  static_assert(t ==1 || t ==2);
}