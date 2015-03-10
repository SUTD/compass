using namespace spec;

void foo()
{
  vector<vector<int> > v;

  for(int i=0; i<10; i++)
  {
    vector<int> inner;
    for(int j=0; j<4; j++) {
	inner.push_back(j);
    }
    v.push_back(inner);

  }

  static_assert(v[0][0] == 0);
  static_assert(v[1][0] == 0);
  static_assert(v[0][1] == 1);

 

  
}