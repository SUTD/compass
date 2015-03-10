

void test()
{

  int x;
  x=3;
  x= x;
  static_assert(x == 3);

}