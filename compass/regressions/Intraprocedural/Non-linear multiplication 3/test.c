#include <stdio.h>

void foo(int a, int b, int c)
{
  int x = a*b;
  int y = b*c;
  static_assert(x==y); //should fail
  
}