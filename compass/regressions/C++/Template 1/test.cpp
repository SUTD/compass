#include <stdlib.h>

template <class T>
void foo(T in)
{
  static_assert(in == NULL);
}

void test_foo()
{
  foo<int>(0);
  foo<int*>(NULL);
}