#include <stdlib.h>

class Foo
{
  int x;
public:
  Foo(){x = 5;};
  virtual int get_x() const {return x;};
};

void test_reference()
{
  Foo x;
  int res = x.get_x();
  static_assert(res == 5); 
}