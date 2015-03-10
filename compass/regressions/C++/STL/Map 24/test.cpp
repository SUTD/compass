using namespace spec;



int boo(map<int, int>::iterator it)
{
  int x = it->second;
  return x;
}

void foo(int k)
{
  map<int, int> m;
  m[0] = 5;
  m[6] = 7;
  map<int, int>::iterator it = m.begin();
  int z = boo(it);
  static_assert(z>5); //should fail

  
 



}