void specify_checks()
{
  check_deleted_access();
}

class A
{

public:
  int x;
 // int y;
  A(int b){ x =b;};
  virtual int foo() {return 1;};
  //~A(){};
  
};



void bar(A** x, int size)
{
  //for(int i=0; i<size; i++)
   // delete x[0];
}

void foo()
{	

  A** arr = new A*[3];
  arr[1] = 0; //no error


}