void specify_checks()
{
  check_deleted_access();
//check_double_delete();
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
 
  for(int i=0; i<size; i++)
     delete x[i];
}

void foo()
{	

  A* arr[2];
  A* a = new A(2);
  arr[0] = a;
  arr[1] = new A(3);

  bar(arr, 2); 

  int t = a->x; //error

  


}