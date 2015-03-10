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
  int get_x()
  {
    return x;
  }
};



void foo(int size)
{
  vector<A*> a_vec;

  for(unsigned int i=0; i<size; i++)
  {
    A* a = new A(i);
    a_vec.push_back(a);
  }

  vector<A*>::iterator it = a_vec.begin();
  it++;
  for(; it!= a_vec.end(); it++)
  {	
    delete *it;
  }
  // first element is leaked

  
}