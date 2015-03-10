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
  A* a = new A(0);
  a_vec.push_back(a);
  // leak
  

  
}