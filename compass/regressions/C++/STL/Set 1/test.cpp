
using namespace spec;

void test()
{

  set<int> s;
  s.insert(1);
  s.insert(44);
 static_assert(s.count(1) > 0);
  static_assert(s.count(44) > 0);

}