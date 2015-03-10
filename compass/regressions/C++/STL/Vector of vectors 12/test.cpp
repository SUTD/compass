using namespace spec;

void foo()
{
  vector<vector<int> > v;

  vector<int> w1;
  w1.push_back(1);
  w1.push_back(2);

  vector<int> w2;
  w2.push_back(3);
  w2.push_back(4);

  v.push_back(w1);
 v.push_back(w2);

  static_assert(v[0][0] == 1);
  static_assert(v[0][1] == 2);
  static_assert(v[1][0] == 3);
  static_assert(v[1][1] == 4);

  
}