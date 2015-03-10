#include <stdio.h>

void foo()
{
  static_assert(1);
}

void baz()
{

}

void bar()
{
  void (*a)() = &foo;
  void (*b)() = &baz;
  static_assert(*a != *b);
    
}