void test(int* a)
{
  alias1(a, a, a, a, a);
}

void alias1(int* x, int* y, int* z, int* w, int* q)
{
  *x= 6;
  *y= 7;
  *z = 8;
  *w = 9;
  *q = 10;
  static_assert(*w == 9); //should fail
  
}