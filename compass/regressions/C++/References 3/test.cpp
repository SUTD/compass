#include <stdlib.h>

void test_reference()
{
  int x = 4;
  const int & y = x;
  static_assert(x == 4);
}