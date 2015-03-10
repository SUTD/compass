void specify_checks()
{
  check_cast();
};

class A
{
  public:
  A() {};
  virtual int foo() {return 1;}

};

class B: public A
{
  public:
  B(){};
  virtual int foo() {return 2;}
};

void test(B* b)
{
  int x = b->foo(); 
}

void bar()
{
  A* a = new A();
  test((B*) a); //should fail!
}