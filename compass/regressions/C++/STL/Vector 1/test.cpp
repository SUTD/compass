using namespace spec;

void foo()
{
  vector<int> a;
  a.push_back(3);
  int z = a[0];
  static_assert(z == 3);


  vector<int*> b;
  int x = 55;
  int*y = &x;
  b.push_back(y);
  *b[0] = 1;
  int c = *b[0];
  static_assert(c == 1);

  
}