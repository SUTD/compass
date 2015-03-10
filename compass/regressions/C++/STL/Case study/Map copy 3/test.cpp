
using namespace spec;


void bar()
{
  map<string, int> m;
  m["aa"] = 3;
  m["bb"] = 9;
  m["cc"] = 34;


  map<string, int> m2;

  map<string, int>::iterator it = m.begin();
  for(; it!= m.end(); it++)
  {
    string k= it->first;
    int v = it->second;
    m2[k] = v;
  }

  it = m.begin();
  for(; it!= m.end(); it++)
  {
    string k1= it->first;
    int v2 = it->second;
    static_assert(m2[k1] == v2);
  }


}