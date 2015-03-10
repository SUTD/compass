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

int rand();

A* foo(A* a)
{	

  if(rand()) return a;
  return new A(4);
}

void test()
{
    A* a = new A(2);
    A* res = foo(a);
    delete res; 
    delete a;	//error!

}