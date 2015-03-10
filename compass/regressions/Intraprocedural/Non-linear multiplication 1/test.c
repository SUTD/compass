#include <stdio.h>

void foo(int a, int b)
{
  int x = a*b;
  int y = a*b;
  static_assert(x==y);
  
}