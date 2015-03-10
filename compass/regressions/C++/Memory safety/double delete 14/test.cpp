void specify_checks()
{
  check_double_delete();
}

class A
{
  int x;
public:
  A() {this->x=0;}
  A(int b){this->x = b;};

  ~A(){};
  
};

void foo()
{
  A* a = new A[4];
  A* x = a;
  delete a;
  delete x; //should fail
}