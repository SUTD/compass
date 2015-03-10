#include <stdlib.h>

class Foo
{
  int p;
  int q;
  public:
  Foo() {p=7;q=3;};
  virtual int get_sum() {return p+q;}
  ~Foo(){}
};

Foo* foo()
{
  Foo* a = new Foo[5];

  static_assert(a[4].get_sum() == 10);
  return a;
}

