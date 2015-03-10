
using namespace spec;


void bar()
{
  map<int, int> m;
  m[3] = 4;
  m[4] = 34;
  m[5] = 23;
  m[43] = 24;
  m[54] = 56;


  map<int, int> reverse_m;

  map<int, int>::iterator it = m.begin();
  for(; it!= m.end(); it++)
  {
    int k= it->first;
    int v = it->second;
    reverse_m[v] = k;
  }

  it = m.begin();
  for(; it!= m.end(); it++)
  {
    int k1= it->first;
    int v2 = it->second;
    static_assert(reverse_m[k1] == v2);
  }


}