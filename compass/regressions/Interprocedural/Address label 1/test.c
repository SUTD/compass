#include <stdio.h>

void foo()
{
  static_assert(1);
}

void baz()
{

}


void call(void (*a)(), void (*b)(), int flag)
{
 if(!flag) static_assert(*a == *b); 
}

void bar(int flag)
{
  void (*a)() = &foo;
  void (*b)();
  if(flag) b = &baz;
  else b = &foo;
  call(a, b, flag);
 
    
}

