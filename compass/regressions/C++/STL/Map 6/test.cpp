using namespace spec;

void foo(int k)
{
  map<int, int> m;
  m[4] = 3;
  int z = m[7];
  int s = m.size();
  static_assert(s == 2); 
 



}