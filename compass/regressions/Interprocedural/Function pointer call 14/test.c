#include <stdio.h>

void foo(int* x)
{
  *x = 1;
}

void bar(int* x)
{
  *x = 2;
}

void  baz(int* x)
{
  return 3;
}

typedef void (*ptFn)(int*);



int test(int t)
{

  int x;
  ptFn p = NULL;
  if(t < 0) p = &foo;
  else if(t==0) p = &bar;
  else p = &baz;
  
  call(p, &x);

  if(t<0) static_assert(x==1); //correct

}

void call(ptFn p, int* x)
{
  (*p)(x);
}