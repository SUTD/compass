void specify_checks()
{
  check_deleted_access();
}

class A
{
  int x;
public:
  A(int b){this->x = b;};

  ~A(){};
  
};



void foo()
{	
  A* a = new A(2);

  A* arr[2];
  arr[0] = new A(1);
  arr[1] = new A(2);
   arr[2] = new A(2);

  for(int i=0; i<3; i++)
    delete arr[i];

  A b = *a;  //ok

}