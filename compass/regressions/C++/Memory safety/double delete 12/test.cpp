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
    A* x =  *a;
    delete x;
}


void bar(int flag)
{
  A* a = foo();
  if(flag) a = new A(2);
  else test(&a);
  
  if(flag) delete a; //ok
}