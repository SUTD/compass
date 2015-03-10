
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
  B* b = new C();
  C* c = static_cast<C*>(b);
  static_assert(c->foo() == 3);
  static_assert(c->bar() == 2); //should fail
  
}