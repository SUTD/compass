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

void foo()
{	
  A* a = new A(2);
  delete a; //no error


}