using namespace spec;



int boo(map<int, int>::iterator it)
{
  int x = it->second;

  //static_assert(x > 0);

  return x;
}

void foo(int k)
{
  map<int, int> m;
  m[0] = 5;
  m[6] = 7;
  map<int, int>::iterator it = m.begin();
  int z = boo(it);
  static_assert(z>4);
  //int x = it->second;
  //static_assert(x > 0);

  /*
  for(; it != m.end(); it++)
  {
    int v = it->second;
    static_assert(v > 0);
  }*/
  
 



}