#include <stdio.h>

int foo()
{
  return 1;
}

int bar()
{
  return 2;
}

int baz()
{
  return 3;
}

typedef int (*ptFn)();

int test(int flag)
{
  
  ptFn a[] = {&baz, &bar, &foo};
  int b = (*a[0])();
  static_assert(b==3);
  int c = (*a[1])();
  static_assert(c==2);
  int d = (*a[2])();
  static_assert(d==1);
  

}