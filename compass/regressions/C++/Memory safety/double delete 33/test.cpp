void specify_checks()
{
  check_double_delete();
}

class B
{
  public:
  int x;
  B() {x=0;}
};


class A
{
  public:

  B* b;
  A(B* b) {this->b = b;}
  A() {this->b = 0;}
 // ~A(){delete b;};
};


void foo(int flag)
{
  A** a = new A*[3];
  A** b = new A*[3];
  a[0] = new A();
  a[1] =  a[2] = new A();
  b[0] = new A();
  b[1] = new A();
  b[2] = new A();

  A** c;
  if(flag) c=a;
  else c = b;
   

  if(!flag)
  {
    for(int i=0; i<3; i++)
    {
      delete c[i]; //ok
    }
  }
}