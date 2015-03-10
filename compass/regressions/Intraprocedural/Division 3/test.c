#include <stdio.h>

void foo(int a, int b, int c)
{
  int x = a/2;
  int y = a/2;
  static_assert(x==y);  
  
}