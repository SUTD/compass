struct bar
{

  int f;
  int g;
};




void test()
{
  struct bar s;
  int* x = &s.g;
  int r = alias1(&s, x);
  static_assert(!r); //should fail
}

int alias1(struct bar* s, int* x)
{
  s->g = 6;
  *x = 7;
  return  (s->g == *x);
}
