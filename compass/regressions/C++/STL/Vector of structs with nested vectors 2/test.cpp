using namespace spec;


struct ss
{
  int x;
  vector<int> inner_v;
};

void foo()
{
  vector<ss> v;
  ss s;
  
  s.x = 89;
  s.inner_v.push_back(8);
  s.inner_v.push_back(9);
  
  v.push_back(s);
  int zz = v[0].inner_v[1];
  static_assert(zz == 8); //should fail


  
}