#include <stdlib.h>

class Foo
{
  int p;
  int q;
  public:
  Foo() {p=7;q=3;};
  ~Foo(){}
};

Foo* foo()
{
  Foo* a = new Foo[5];
  return a;
}

