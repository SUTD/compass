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
  A* a = new B();
  B* b = dynamic_cast<B*>(a);
  static_assert(b->foo() == 2); 
}