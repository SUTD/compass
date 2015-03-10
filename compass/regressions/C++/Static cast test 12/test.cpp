
class A
{
  public:
  int x;
  A() {x = 1;}
  virtual int foo() {return 1;}

};

class B
{
  public:
  int x;
  B() {x = 2;}
  virtual int bar() {return 2;}
};

class C: public A, public B
{
  public:
  C() {};
  virtual int foo() {return 3;}
  virtual int bar() {return 4;}
};



void test()
{
  C* c = new C();
  B* b = static_cast<B*>(c);
  static_assert(b->bar() == 4); //correct
  static_assert(b->x == 1); //should fail
  
}