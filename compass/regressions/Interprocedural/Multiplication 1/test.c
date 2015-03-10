#include <stdio.h>

int foo(int a, int b)
{
    return a*b;
  
}

int bar(int x)
{
  return x*2;
}

void baz()
{
  int y = bar(8);
  static_assert(y == 16);
}