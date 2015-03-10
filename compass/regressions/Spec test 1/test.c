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
  static_assert(foo() == 1);

}