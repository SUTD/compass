#include <stdio.h>

void foo(int a, int b)
{
  int x = a*b;
  int y = b*a;
  static_assert(x==y);
  
}