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

  ~B()
  {
    //delete a;
  }

};

void foo()
{
  B* b = new B(3);
  delete b; // a inside b is leaked
}