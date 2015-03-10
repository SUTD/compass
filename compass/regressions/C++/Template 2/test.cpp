#include <stdlib.h>

template <class T>
T get_ith(T* in, int i)
{
  return in[i];
}

void test_foo()
{
  int a[3];
  a[0] = 0;
  a[1] = 1;
  a[2] = 2;
  int x = get_ith(a, 2);
  static_assert(x == 2);
}