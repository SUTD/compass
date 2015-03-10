
/*
 * We insert every key,value pair in map m
 * in another map m2, and assert that m2 contains
 * every key in m and that the value associated with this key
 * are the same in m and m2.
 */

using namespace spec;


void bar()
{
  map<int, string> m;
  m[3] = "aa";
  m[4] = "bb";
  m[5] = "cc";
  m[43] = "ff";
  m[54] = "kl";


  map<int, string> m2;

  map<int, string>::iterator it = m.begin();
  for(; it!= m.end(); it++)
  {
    int k= it->first;
    string v = it->second;
    m2[k] = v;
  }

  it = m.begin();
  for(; it!= m.end(); it++)
  {
    int k1= it->first;
    string v2 = it->second;
    static_assert(m2[k1] == v2);
  }


}