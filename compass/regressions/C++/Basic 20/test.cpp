namespace ns1
{
  int foo() {return 1;}
}

namespace ns2 
{
  int foo() {return 2;}
}

void bar()
{
  int x = ns1::foo();
  int y = ns2::foo();
  static_assert(x == 1);
  static_assert(y == 2);
}