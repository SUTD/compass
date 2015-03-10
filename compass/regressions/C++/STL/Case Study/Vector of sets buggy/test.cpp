
using namespace spec;



/*
 * We build a vector of sets (some of which are aliases), 
 * insert and erase various elements in the nested sets,
 * and assert properties that should hold about elements
 * of nested sets.
*/


void test()
{

  vector<set<int>* > v;

  set<int>* s1 = new set<int>();
  s1->insert(2);
  s1->insert(9);
  s1->insert(24);
  s1->insert(11);

  set<int>* s2 = new set<int>();
  s2->insert(32);
  s2->insert(11);
  s2->insert(45);
  s2->insert(23);


  v.push_back(s1);
  v.push_back(s2);
  v.push_back(s2);

  static_assert(v.size() == 3);
  static_assert(v[0]->count(9)>0);

  v[2]->erase(11);

  int num =  v[1]->count(11);
  static_assert(num > 0); //wrong! v[1] and v[2] are aliases
  static_assert(v[1]->count(45) > 0);
  static_assert(v[0]->count(11) > 0);


}