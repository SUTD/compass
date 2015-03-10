#include <stdlib.h>

template <class T>
class Foo
{
  public:
  T elem;

  Foo(T t):elem(t){};

  T get_t() {return elem;};
};

void test()
{

  int x = 3;
  Foo<int>* f = new Foo<int>(x);
  static_assert(f->get_t() == 2); //should fail
}