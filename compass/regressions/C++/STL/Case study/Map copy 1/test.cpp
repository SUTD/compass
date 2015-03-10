
using namespace spec;


void bar()
{
  map<int, int> m;
  m[4] = 3;
  m[5] = 9;

  map<int, int> m2;

  map<int, int>::iterator it = m.begin();
  for(; it!= m.end(); it++)
  {
    int k= it->first;
    int v = it->second;
    m2[k] = v;
  }

  int v1 = m2[4];
 int v2 = m2[5];
  static_assert(v1==3);
  static_assert(v2 == 9);
}