using namespace spec;





void foo(int k)
{
  map<int, int> m;
  m[0] = 5;
  m[6] = 7;
  map<int, int>::iterator it = m.begin();
  int i = 0;
  for(; it != m.end(); it++, i++)
  {
     int x = it->second;
     static_assert(x>=5);
  }
  static_assert(i == 3); //should fail

}