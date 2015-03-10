#include <stdlib.h>

class Foo
{

};

void foo()
{
  Foo* a = new Foo;
  static_assert(a!=NULL);
}