
class A
{
  public:
  A();
  virtual int foo() {return 1;}

};

class B: public A
{
  public:
  B();
  virtual int foo() {return 2;}
};

void test()
{
  A* a = new B();
  B* b = static_cast<B*>(a);
  static_assert(b->foo() == 2);
  static_assert(a->foo() == 1); //should fail
}