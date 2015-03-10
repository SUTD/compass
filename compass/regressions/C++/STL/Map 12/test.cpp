using namespace spec;

void foo(int k)
{
  map<int, int> m;
  m[4] = 3;
  int z = m[7];
  m.erase(k);
  
  int s = m.size();
  if(k == 88)
    static_assert(s == 2);
 



}