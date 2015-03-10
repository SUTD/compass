using namespace spec;

void foo(int k)
{
  map<int, int> m;
  m[4] = 3;
  int x = m[4];
  static_assert(x == 3);
 



}