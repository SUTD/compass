#include <stdlib.h>

class Foo
{
  int p;
  int q;
  public:
  Foo() {p=7;q=3;};
  virtual int get_sum() {return p+q;}
  virtual ~Foo(){}
};

Foo* foo()
{
  Foo* a = new Foo[5];
  return a;
}

