using namespace spec;

void foo(int k)
{
  map<int, int> m;
  m[4] = 3;
  int z = m[7];
  m.erase(k);
  
  int s = m.size();
  int elem = m[4];
  if(k!=4)
    static_assert(elem == 3);
 



}