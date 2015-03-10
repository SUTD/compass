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

void delete_array(A** arr, int size)
{
  for(int i=0; i<size; i++)
  {
    delete arr[i];
  }
}

C** copy_c_field(A** a_arr, int size)
{  
  C** c_arr = new C*[size];
  
  for(int i=0; i<size; i++)
  {
    c_arr[i] = a_arr[i]->getB()->getC();
  }

  return c_arr;


}

void init_c_field(C** c_arr, int size)
{
  for(int i=0; i<size; i++)
  {
    c_arr[i]->c = 2; 
  } 
}


C** foo(A** a_arr, int size)
{

  
  C** c_arr = copy_c_field(a_arr, size);
  init_c_field(c_arr, size);
  delete_array(a_arr, size);
  return c_arr; //ok


}