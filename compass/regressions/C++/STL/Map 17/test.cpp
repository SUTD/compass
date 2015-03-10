using namespace spec;

void foo(int k)
{
  map<int, int> m;
  m[0] = 5;
  m[1] = 7;

  map<int, int>::iterator it = m.begin();
  int v = it->second;
  static_assert(v==5); //should fail
 



}