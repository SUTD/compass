using namespace spec;

void foo()
{
  vector<vector<int> > v;
  vector<int> w;
  w.push_back(1);
  w.push_back(2);
  v.push_back(w);
  
  vector<int> y;
  y.push_back(3);
  static_assert(v[0][0] == 1); 
  v[0] = y;
  static_assert(v[0][0] == 3); 

  vector<vector<int> >::iterator it = v.begin();
  
  vector<int>::iterator it2 = (*it).begin();

  int x = *it2;

  static_assert(x == 1); //error

 

  
}