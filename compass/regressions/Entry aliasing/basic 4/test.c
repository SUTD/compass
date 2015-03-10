void test(int* a, int* b)
{
    int* c = b;
  alias1(a, b, c);
}

void alias1(int* x, int* y, int* z)
{
  *x= 6;
  *y= 7;
  *z = 8;
  static_assert(*y == 7);  //should fail
}