using namespace spec;


struct inner_elem
{
  int z;
  int w;
};

struct elem
{
  int x;
  inner_elem y;
};

int boo(vector<elem>& v)
{
 return (v[0].y.w);
}

void foo()
{
  vector<elem> v;
  elem e;
  e.x = 1;
  e.y.z = 6;
  e.y.w = 5;
  v.push_back(e);

  static_assert(boo(v) == 6);

}