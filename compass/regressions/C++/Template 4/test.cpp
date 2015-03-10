#include <stdlib.h>

template <class T>
T get_ith(T* in, int i)
{
  return in[i];
}


struct object
{
  int h;
  int w;
  object(){h = 3; w = 3;};
};

void test_foo()
{
  int a[3];
  a[0] = 0;
  a[1] = 1;
  a[2] = 2;
  int x = get_ith(a, 2);

  object o[2];
  o[1].w = 7;
  object y = get_ith(o, 1);
  static_assert(y.w == 7);
  static_assert(y.h == 3);
  
  

  

}