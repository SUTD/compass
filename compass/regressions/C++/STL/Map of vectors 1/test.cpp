using namespace spec;

void foo()
{
  map<int, vector<int> > m;
  vector<int> v;
  v.push_back(1);
  v.push_back(2);
  m[9]=v;
  
  int xx = m[9][0];
  int yy = m[9][1];
  static_assert(xx == 1);
  static_assert(yy == 2);

  static_assert(xx == 3); //should fail
  static_assert(yy == 4); //should fail

  
}