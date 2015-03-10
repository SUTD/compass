using namespace spec;

void specify_checks()
{
  check_deleted_access();
  check_memory_leak();
}

class B
{
  public:
  int x;
  B(int x)
  {
    this->x = x;
  }

  int get_x()
  {
    return x;
  }
};

class A
{
  public:
  vector<B*> b_vec;
  A()
  {
  }
  
  void addB(B* b)
  {
    b_vec.push_back(b);
  }
  
};



void foo(int size)
{
  vector<A*>* a_vec = new vector<A*>();

  for(unsigned int i=0; i<size; i++)
  {
    A* a = new A();

    for(unsigned int j=0; j<5; j++)
    {
      B* b = new B(0);
      a->addB(b);
    }

    a_vec->push_back(a);
  }

  vector<A*>::iterator it = a_vec->begin();
  for(; it!= a_vec->end(); it++)
  {	
    delete *it;
  } 

  delete a_vec;

  // LOTS of b's are leaked
}