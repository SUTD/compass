int foo()
{
  if(rand()) return 0;
}

void test()
{
  static_assert(foo() == 0); //should fail
}