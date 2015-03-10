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
  A* a2 = new A(1);
  a_vec.push_back(a);
  a_vec.push_back(a2);
  delete a_vec[1];
  int b = a->x; 
  delete a; //no errors



  
}