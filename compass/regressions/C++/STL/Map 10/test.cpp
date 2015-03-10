using namespace spec;

void foo(int k)
{
  map<int, int> m;
  m[4] = 3;
  int z = m[7];
  int g = m[k];
  int s = m.size();
  if(k == 4)
    static_assert(s == 2);
 



}