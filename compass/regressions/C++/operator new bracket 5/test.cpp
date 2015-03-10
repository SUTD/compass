#include <stdlib.h>

class Foo
{
  int p;
  int q;
  public:
  Foo() {p=7;q=3;};
  virtual int get_sum() {return p+q;}
};

Foo* foo(int num)
{
  Foo* a = new Foo[num];
  return a;
}

void test()
{
  Foo* f = foo(6);
  for(int i=0; i<6; i++) {
    static_assert(f[i].get_sum() == 11); //should fail
  }
  
}