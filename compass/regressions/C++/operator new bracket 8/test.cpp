#include <stdlib.h>


class Foo
{
  int p;
  int q;
  public:
  Foo(){this->p=0; this->q = 1;};
  Foo(int p, int q) {this->p=p; this->q=q;};
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
  Foo f2(2, 3);
  
  for(unsigned int i=0; i<6; i++)
  {
    static_assert(f[i].get_sum() == f2.get_sum()); //should fail
  }

 
  
}