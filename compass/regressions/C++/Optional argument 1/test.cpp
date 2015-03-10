#include <stdlib.h>


int foo(int opt = 0)
{
  return opt;
}

void bar()
{
  int x = foo();
  static_assert(x == 0); //correct 
}