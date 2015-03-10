#include <stdlib.h>


void test()
{
  int* p = static_cast<int*>(malloc(100));
  if(!p) return;
  *p =1;
  static_assert(p!=NULL);
  static_assert(*p == 1);
  
}