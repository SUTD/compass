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

void bar(A** a, int size)
{
  for(int i=0; i<size; i++)
  {
    delete a[i]; 
  }
}

void foo()
{
  A* arr[8];
  A* a1 = new A();
  A* a2 = new A();
  A* a3 = new A();
  A* a4 = new A();
  A* a5 = new A();
  A* a6 = new A();
  A* a7 = new A();
  A* a8 = new A();
  arr[0] = a1;
  arr[1] = a2;
  arr[2] = a3;
  arr[3] = a4;
  arr[4] = a5;
  arr[5] = a6;
  arr[6] = a7;
  arr[7] = a8;
  bar(arr, 8); // ok

}