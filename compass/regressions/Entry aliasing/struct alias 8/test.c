struct bar
{
 int g;
  int f;
 
};




void test()
{
  struct bar s1;
  struct bar s2;
  alias1(&s1, &s2);
}


void alias1(struct bar* s1, struct bar* s2)
{
  s1->g = 6;
  s2->g = 7;
  static_assert(s1->g != s2->g); //correct
}