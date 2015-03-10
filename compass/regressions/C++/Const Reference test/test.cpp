


void test(const int & a)
{
  static_assert(a == 3);
}

void foo()
{

  test(3);

}