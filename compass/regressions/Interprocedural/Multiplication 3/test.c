#include <stdio.h>

int foo(int a, int b)
{
    return a*b;
  
}

int bar(int x)
{
  return x*2;
}

void baz(int z)
{
  int y = bar(z);
  static_assert(y == 3*z); // should fail
}