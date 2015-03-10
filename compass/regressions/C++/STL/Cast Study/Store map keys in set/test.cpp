
using namespace spec;


/*
 * We collect all the keys in map m in set keys and
 * assert that s contains exactly the keys in m.
*/
void test()
{

  map<string, int> m;
  m["aa"] = 8;
  m["fg"] = 19;
  m["zzz"] = 98;
  set<string> keys;

  map<string,int>::iterator it = m.begin();
  for(; it!= m.end(); it++)
  {
    string s = it->first;
    keys.insert(s);
  }

  static_assert(keys.count("aa") > 0);
  static_assert(keys.count("fg") > 0);
  static_assert(keys.count("zzz") > 0);
   static_assert(keys.count("abc") == 0);

}