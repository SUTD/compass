using namespace spec;

void specify_checks()
{
  check_deleted_access();
  check_memory_leak();
}

class A
{
  public:
  int x;
  A(int x)
  {
    this->x = x;
  }
};



void foo()
{
  vector<A*> a_vec;
  A* a1 = new A(0);
  A* a2 = new A(0);
  A* a3 = new A(0);
  A* a4 = new A(0);
  a_vec.push_back(a1);
  a_vec.push_back(a2);
  a_vec.push_back(a3);
  a_vec.push_back(a4);

  for(unsigned int i=0; i<3; i++)
  {
    delete a_vec[i];
  }	
  
  // last element leaked
}