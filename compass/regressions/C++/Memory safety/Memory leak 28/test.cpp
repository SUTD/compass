void specify_checks()
{
  check_memory_leak();
}

class C
{
  public:
  int v;
  C(int v)
  {
    this->v = v;
  }
  
};

class A
{
  public:
  C* c;
  A(int x)
  {
    this->c= new C(x);
  }

  ~A()
  {
    delete c;
  }
};

class B
{
  public:
  A** a;
  int total_size;
  int actual_size;

  B(int num)
  {
    a = new A*[num];
    total_size = num;
    actual_size = 0;
  }

  bool addA(A* x)
  {
    if(actual_size >= total_size) return false;
    a[actual_size++] = x;
    return true;
  }


  ~B()
  {
    for(int i=0; i<actual_size; i++)
    {
      delete a[i];
     }
    delete[] a;
  }

};

void foo()
{
  B* b = new B(10);
  b->addA(new A(1));
  b->addA(new A(2));
  delete b;
  // no leaks!
}