#include <stdlib.h>

int foo()
{
  return 5;
}

void test_reference()
{

  const int & res = foo();
  static_assert(res == 6); //should fail 
}


