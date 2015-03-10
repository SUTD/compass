#include <stdlib.h>




void test()
{
  int x = 1;
  int  a1;
  int a0;
  int* res = (a0 = ({a1 = x; }), &a0); 
  static_assert(*res == 1);
}