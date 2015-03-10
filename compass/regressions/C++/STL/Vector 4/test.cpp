using namespace spec;

void foo()
{
  vector<int> a;
  a.push_back(3);
  a[1] = 5;
  static_assert(a[1] == 3); //should fail

  
}