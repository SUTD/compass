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
    delete a[i]; 
  }
}

void foo()
{
  A* arr[2];
  A* a = new A();
  arr[0] = a;
  arr[1] = new A();
  bar(arr, 2); //ok

}