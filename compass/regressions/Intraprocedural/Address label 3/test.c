#include <stdio.h>

void foo()
{
  static_assert(1);
}

void baz()
{

}

void bar(int flag)
{
  void (*a)() = &foo;
  void (*b)();
  if(flag) b = &baz;
  else b = &foo;
  static_assert(*a == *b); //should fail
    
}