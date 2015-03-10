
using namespace spec;

void test()
{
  vector<int> v1;
  v1.push_back(4);
  v1.push_back(67);
  v1.push_back(23);
  v1.push_back(54);
  v1.push_back(2);

  vector<int> v2;
  
  vector<int>::iterator it = v1.begin();
  for(; it!= v1.end(); it++)
  {
    int cur = *it;
    v2.push_back(cur);
  }

  it = v1.begin();
  vector<int>::iterator it2 = v2.begin();
  it++;
  for(; it!= v1.end(); it++, it2++)
  {
      int v1_elem = *it;
      int v2_elem = *it2;
  
      static_assert(v1_elem == v2_elem);
  }
  

}