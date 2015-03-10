
using namespace spec;

/*
  * For map m, we build a reverse map 
  * reverse_m such that for each key k that
  * is associated with value v in m, reverse_m
  * containg an entry with key v and value k.
 */

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
    static_assert(reverse_m[v2] == k1);
  }


}