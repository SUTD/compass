int foo()
{
  if(rand()) return 0;
  else return 1;
}

void test()
{
  static_assert(foo() == 0); //should fail
}