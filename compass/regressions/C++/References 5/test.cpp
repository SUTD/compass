#include <stdlib.h>

class Foo
{
  int x;
public:
  Foo(){x = 5;};
  virtual int get_x() {return x;};
};

void test_reference()
{
  Foo x;
  Foo & y = x;
  static_assert(y.get_x() == 6); //should fail
}