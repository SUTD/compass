using namespace spec;





void foo(int k)
{
  map<int, int> m;
  m[0] = 5;
  m[6] = 7;
  map<int, int>::iterator it = m.begin();
  for(; it != m.end(); it++)
  {
     int x = it->second;
     static_assert(x>5); //should fail
  }

}