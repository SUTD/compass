using namespace spec;

void foo(int k)
{
  map<int, int> m;
  m[3] = 88;
  static_assert(m[3] == 88);
  int u = m[6];
  static_assert(u == 0);

  int z = m.size();
  static_assert(z == 2);
 
  m.erase(6);
  z = m.size();
  static_assert(z == 1);

  bool h = m.count(3) > 0;
  static_assert(h);
 



}