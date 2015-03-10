
using namespace spec;

void test()
{

  set<int> s;
  s.insert(1);
  s.insert(44);
  s.erase(1);
  s.insert(5);

  int x = *s.begin();
  static_assert(x == 44); //should fail

}