using namespace spec;



void boo(map<int, int>::iterator it)
{
  int x = it->second;
  static_assert(x >=5);
}

void foo(int k)
{
  map<int, int> m;
  m[0] = 5;
  m[6] = 7;
  map<int, int>::iterator it = m.begin();
  boo(it);

}