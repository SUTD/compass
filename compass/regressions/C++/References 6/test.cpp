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
  const Foo & y = x;
  static_assert(y.get_x() == 5); 
}