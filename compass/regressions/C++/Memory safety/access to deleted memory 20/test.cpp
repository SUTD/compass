void specify_checks()
{
  check_deleted_access();
}

class B
{
public:
  int a;
  B(){a=2;};
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
 // virtual int foo() {return 1;};
  ~A(){delete b;};
  
};



void foo()
{	
  A* a = new A(4);
  B* b = a->getB();
  delete a;
  int x = b->a;

}