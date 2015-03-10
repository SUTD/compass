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
  A* a[3];
  A* x = new A(1);
  A* y = new A(2);
  A* z = new A(3);
  a[0] = x;
  a[1] = y;
  a[2] = z;

 // A** b = a;

  for(int i=0; i<3; i++)
  {
    delete a[i]; 
    delete a[i];

  }
}