
using namespace spec;

int unknown();

/*
 * We build a map from strings to a list of ints,
 * and assert various properties that should hold 
 * about the nested lists as well as the elements
 * of the nested lists.
*/
void test()
{

  map<string, list<int>* > m;

  list<int>* l1 = new list<int>();
  l1->push_back(3);
  l1->push_front(7);
  l1->push_back(45);

  list<int>* l2 = new list<int>();
  l2->push_back(34);
  l2->push_front(23);
  l2->push_front(2);

  list<int>* l3 = new list<int>();
  l3->push_front(3);
  l3->push_front(56);
  l3->push_back(24);

  m["aa"] = l1;
  m["bb"] = l2;
  m["cc"] = l3;

  m["bb"]->pop_front();
  m["cc"]->pop_back();


  static_assert(l1->size() == 3);
  static_assert(l2->size() == 2);
  static_assert(l3->size() == l2->size());

  static_assert(m["aa"]->front() == 7);
  static_assert(m["bb"]->front() == 23);
 
  int x = m["cc"]->back();
  static_assert(x == 56); //wrong

}