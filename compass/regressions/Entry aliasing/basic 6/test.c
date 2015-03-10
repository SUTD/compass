void test()
{
  int a, b, c;
  alias1(&a, &b, &c);
}

void alias1(int* x, int* y, int* z)
{
  *x= 6;
  *y= 7;
  *z = 8;
  static_assert(*x == 6);  //ok
  static_assert(*y == 7); //ok
}