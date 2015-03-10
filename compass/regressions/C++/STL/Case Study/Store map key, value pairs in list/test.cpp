
using namespace spec;

int unknown();

/*
 * The list kv_pairs stores all the key, value pairs in map m.
 * We check functional correctness by asserting:
 * (i) For every pair<k, v> in list kv_pairs, the value
       associated with key k is indeed v, and
 * (ii) For any key "some_key" that is not in map m,
        the list also does not contain a pair with this key.
*/
void test()
{

  map<int, int> m;
  m[45] = 8;
  m[23] = 19;
  m[454] = 98;
  m[256] = 35;


  list<pair<int, int> > kv_pairs;

  map<int,int>::iterator it = m.begin();
  for(; it!= m.end(); it++)
  {
    int key = it->first;
    int val = it->second;
    pair<int, int> p(key, val);
    kv_pairs.push_front(p);
  }

  list<pair<int, int> >::iterator it2 = kv_pairs.begin();
  for(;it2!= kv_pairs.end(); it2++)
  {
    pair<int, int> p = *it2;
    int k = p.first;
    int v = p.second;
    static_assert(m[k] == v);
  }


  int some_key = unknown();
  if(m.count(some_key) == 0) {

    it2 = kv_pairs.begin();
    for(;it2!= kv_pairs.end(); it2++)
    {
      int k = it->first;
      int v = it->second;

      static_assert(k!=some_key);
    }

  }

}