#include <stdio.h>

void shift(int* a, int n, int pos)
{
  int i;
  for(i=pos; i<n-1; i++)
  {
    a[i] = a[i+1];
  }
}

void foo()
{
  int a[4];
  a[0] = 0;
  a[1] = 1;
  a[2] = 2;
  a[3] = 3;
  shift(a, 4, 1);
  static_assert(a[0] == 0);
  static_assert(a[1] == 2);
  static_assert(a[2] == 3);
  static_assert(a[3] == 3);
}