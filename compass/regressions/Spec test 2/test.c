#include <stdlib.h>



int foo()
{
  return 0;

}

SPEC(foo, int)
{
  return 1;
}


void bar()
{
  //should fail
  static_assert(foo() == 0);

}