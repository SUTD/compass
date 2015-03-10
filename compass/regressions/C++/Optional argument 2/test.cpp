#include <stdlib.h>


int foo(int opt = 0)
{
  return opt;
}

void bar()
{
  int x = foo(2);
  static_assert(x == 2); 
}