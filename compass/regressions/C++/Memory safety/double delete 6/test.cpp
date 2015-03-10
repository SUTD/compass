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

A* foo(A* a)
{	

  return a;
}

void test()
{
    A* a = new A(2);
    A* res = foo(a);
    delete a;
    delete res; //error!
}