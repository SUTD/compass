struct bar
{
 int g;
  int f;
 
};




void test()
{
  struct bar s;
  int* x = &s.g;
  int r = alias1(&s, x);
  static_assert(r); //should fail
}

void store(int* a, int b)
{
  *a = b;
}

int alias1(struct bar* s, int* x)
{
  s->g = 6;
  store(x, 7);
  return  (s->g != *x);
}