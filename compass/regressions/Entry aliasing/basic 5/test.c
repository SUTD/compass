void test(int* a, int* b)
{
    int* c = a;
  alias1(a, b, c);
}

void alias1(int* x, int* y, int* z)
{
  *x= 6;
  *y= 7;
  *z = 8;
  static_assert(*x == 6);  //should fail
}