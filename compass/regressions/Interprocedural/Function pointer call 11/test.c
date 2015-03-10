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

  if(t<0) static_assert(x==2); //should fail

}

int call(ptFn p)
{
  return (*p)();
}