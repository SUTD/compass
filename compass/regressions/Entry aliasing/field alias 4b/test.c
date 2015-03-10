struct bar
{

  int f;
  int g;
};


void store(int* a, int b)
{
  *a = b;
}

void test(struct bar* s, int* x)
{
  s->g = 6;
  store(x, 7);
  static_assert  (s->g == *x); 
}

void foo(struct bar* s)
{

  int* x;
  x = &s->g;
  test(s, x);
  
}