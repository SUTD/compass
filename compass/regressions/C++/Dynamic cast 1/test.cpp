#include <stdlib.h>


class A
{
  public:
  A(){};
  virtual int foo(){ return 1;}
};

class B:public A
{
  public:
  B(){};
  virtual int foo() {return 2;}
};

void test()
{
  B* b = new B();
  A* a = dynamic_cast<A*>(b);
  static_assert(a->foo() == 2);
}