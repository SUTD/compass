using namespace spec;

void foo()
{
  vector<vector<int> > v;
  vector<int> w;
  w.push_back(1);
  w.push_back(2);
  v.push_back(w);
  
  w[0] =4;
  static_assert(w[0] == 4);
  static_assert(v[0][0] == 1);
 

  
}