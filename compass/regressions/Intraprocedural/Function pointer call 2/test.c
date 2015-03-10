#include <stdio.h>

int foo()
{
  return 1;
}

int bar()
{
  return 2;
}

int baz()
{
  return 3;
}


int main(int argc, char** argv)
{
  
  int (*a)() = &foo;
  
  int res1 = (*a)();
  static_assert(res1 == 2); // should fail

}