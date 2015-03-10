#include <stdio.h>

void foo(int a, int b, int c)
{
  int x = (a*b)/2;
  int y = (b*a)/2;
  static_assert(x==y);  
  
}