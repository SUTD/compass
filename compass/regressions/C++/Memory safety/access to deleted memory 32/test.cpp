void specify_checks()
{
  check_deleted_access();
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
    //delete this->a;
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
  A* a = b->getA();
  b->setA(new A(1));
  int c = a->x; // ok
  delete b;
  delete a;
  
}