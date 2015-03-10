struct bar
{

  int f;
  int g;
};

void test(struct bar* s, int* x)
{
  s->g = 6;
  *x = 7;
  static_assert  (s->g != *x); //should fail
}

void foo(struct bar* s)
{

  int* x;
  x = &s->g;
  test(s, x);
  
}



