using namespace spec;

void foo(int k)
{
  map<int, int> m;
  m[0] = 5;
  m[1] = 7;



  map<int, int>::iterator it = m.begin();

  int v = it->second;
  static_assert(v>0);
  it++;
  int v2 = it->second;
  static_assert(v2 >0);
  if(v == 5) static_assert(v2 == 7);
 /* for(; it != m.end(); it++)
  {
    int v = it->second;
    static_assert(v > 0);
  }*/
  
 



}