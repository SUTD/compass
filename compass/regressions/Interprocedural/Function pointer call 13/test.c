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



int test(int t)
{
  ptFn p = NULL;
  if(t < 0) p = &foo;
  else if(t==0) p = &bar;
  else p = &baz;
  
  int x = call(p);

  if(x == 1) static_assert(t<0);
  else static_assert(t > 0); // this is wrong!

}

int call(ptFn p)
{
  return (*p)();
}