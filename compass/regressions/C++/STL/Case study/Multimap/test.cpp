
using namespace spec;


void bar()
{
  map<string, set<int> > m;

  m["a"].insert(1);
  m["c"].insert(2);
  m["b"].insert(3);
  m["b"].insert(4);
  m["a"].insert(5);
  m["b"].insert(6);

  static_assert(m["a"].size() == 2);
  static_assert(m["b"].size() == 3);
  static_assert(m["c"].size() == 1);

  static_assert(m["a"].count(1) > 0);
  static_assert(m["b"].count(1) == 0);
  static_assert(m["b"].count(4) > 0);
  static_assert(m["c"].count(2) >0);
  
}