
using namespace spec;

void test()
{

  set<int> s;
  s.insert(1);
  s.insert(44);
  s.erase(1);
  static_assert(s.count(1) == 1); // should fail

}