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



void foo(int size)
{
  vector<A*> a_vec;

  for(unsigned int i=0; i<size; i++)
  {
    A* a = new A(i);
    a_vec.push_back(a);
  }

  if(size > 1) {
    int x = a_vec[0]->x;
  }

  for(unsigned int i=0; i<size; i++)
  {
    delete a_vec[i];
  }

  // no error
  
}