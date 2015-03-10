struct bar
{
 int g;
  int f;
 
};




void test(struct bar* s1)
{
  struct bar* s2 = s1;
  int* x = &s2->g;
  alias1(s1, s2, x);
}


void alias1(struct bar* s1, struct bar* s2, int* z)
{
  s1->g = 6;
  s2->g = 7;
  *z = 8;
  static_assert(s2->g == 7);  //should fail
}