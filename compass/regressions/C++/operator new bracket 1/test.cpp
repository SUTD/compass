#include <stdlib.h>

class Foo
{
  int p;
  int q;
  public:
  Foo() {p=7;q=3;};
  virtual int get_sum() {return p+q;}
};

Foo* foo()
{
  Foo* a = new Foo[5];
  return a;
}

void test()
{
  Foo* f = foo();
  for(int i=0; i<5; i++) {
    static_assert(f[i].get_sum() == 10);
  }
  
}

