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


int test(int flag)
{
  
  int (*a)();
  if(flag) a = &foo;
  else a = &bar;
 
  
  int res1 = (*a)();
  static_assert(res1 == 2); // should fail

}