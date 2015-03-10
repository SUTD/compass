void specify_checks()
{
  check_deleted_access();
}

class A
{

public:
  int x;
  //A(int b){this->x = b;};
  virtual void f();

  ~A(){};
  
};

class B:public A
{
  virtual void f();
};
/*
void bar(A** x, int size)
{
  //for(int i=0; i<size; i++)
   // delete x[0];
}

void foo()
{	
  A* a = new A(2);

  A** arr = new A*[3];
  arr[0] = new A(1);
  arr[1] = new A(2);
   arr[2] = new A(2);

  bar(arr, 3);

  A b = *a;  //ok

}
*/
void foo2(A* a)
{
  int y = a->x; //ok
}