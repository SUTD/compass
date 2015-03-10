void specify_checks()
{
  check_deleted_access();
}

class C
{
  public:
  int c;
  C() {c=7;}
 virtual int ccc() {return 1;};
};

class B
{
public:
  int a;
  C* c;
  B(){a=2; c = new C();};
  ~B() {delete c;}
  virtual int bar() {return 1;};
  C* getC(){return c;};
  
};

class A
{

public:
  int x;
  B* b;
  A(int c)
  { 
    x =c;
    b = new B();
  
  };
  B* getB() {return b;};
  virtual int foo() {return 1;};
  ~A(){delete b;};
  
};



void foo()
{	
  A* a = new A(4);
  C* c= a->getB()->getC();
  delete a;
  int x = c->c; //error

}