int foo()
{

}

void test()
{
  static_assert(foo() == 0); //should fail
}