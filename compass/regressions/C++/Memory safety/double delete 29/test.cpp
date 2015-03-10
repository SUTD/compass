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
};

void bar(A** a, int size)
{
  for(int i=0; i<size; i++)
  {
    delete a[i]->b; 
  }
}

void foo()
{
  A* arr[2];
  B* b = new B();
  A* a1 = new A(b);
  A* a2 = new A(new B());
  arr[0] = a1;
  arr[1] = a2;
  bar(arr, 2); // ok

}