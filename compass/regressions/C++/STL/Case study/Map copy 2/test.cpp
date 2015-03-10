
using namespace spec;


void bar()
{
  map<int, int> m;
  m[4] = 3;
  m[5] = 9;
  m[34] = 23;
  m[98] = 133;
  m[45] = 32;

  map<int, int> m2;

  map<int, int>::iterator it = m.begin();
  for(; it!= m.end(); it++)
  {
    int k= it->first;
    int v = it->second;
    m2[k] = v;
  }

  it = m.begin();
  for(; it!= m.end(); it++)
  {
    int k1= it->first;
    int v2 = it->second;
    static_assert(m2[k1] == v2);
  }


}