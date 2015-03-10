
class A
{
  public:
  int x;
  A();
  virtual int foo() {return 1;}

};

class B
{
  public:
  int y;
  B();
  virtual int bar() {return 2;}
};

class C: public A, public B
{
  public:
  C();
  virtual int foo() {return 3;}
  virtual int bar() {return 4;}
};



void test()
{
  A* a = new C();
  C* c = static_cast<C*>(a);
  static_assert(c->foo() == 3);
}