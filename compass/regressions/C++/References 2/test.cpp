#include <stdlib.h>

void test_reference()
{
  int x = 4;
  int & y = x;
  y = 2;
  static_assert(x == 4); //should fail
}