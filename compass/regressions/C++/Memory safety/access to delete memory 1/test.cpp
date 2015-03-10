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
  delete a;

  A b = *a; //error

}