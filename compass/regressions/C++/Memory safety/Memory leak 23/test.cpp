void specify_checks()
{
  check_memory_leak();
}


class A
{
  public:
  int x;
  A(int x)
  {
    this->x=x;
  }
};

class B
{
  public:
  A* a;
  B(int x)
  {
    this->a = new A(x);
  }

  A* getA()
  {
    return a;
  }

  void setA(A* a)
  {
    this->a = a;
  }

  ~B()
  {
    delete a;
  }

};

void foo()
{
  B* b = new B(3);
  b->setA(new A(1));
  delete b;
  // a allocated in b's constructor is leaked
  
}