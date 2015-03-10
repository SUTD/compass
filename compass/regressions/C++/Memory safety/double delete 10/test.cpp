void specify_checks()
{
  check_double_delete();
}

class A
{
  int x;
public:
  A(int b){this->x = b;};

  ~A(){};
  
};


A* foo()
{	
  return new A(4);
}

void test(A** a)
{
    A* x = *a;
    delete x;
}


void bar()
{
  A* a = foo();
  test(&a);
  delete a; //error
}