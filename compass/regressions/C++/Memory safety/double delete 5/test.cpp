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

void foo(int flag)
{	
  A* a = new A(2);
  A* x;
  if(flag) x = a;
  
  // ,,,
  
 if(flag) delete x;
 delete a; // error


}