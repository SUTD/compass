void specify_checks()
{
  check_memory_leak();
}


class A
{
  public:
  int x;
  A(int x)
  {
    this->x=x;
  }
};

class B
{
  public:
  A** a;
  int total_size;
  int actual_size;

  B(int num)
  {
    a = new A*[num];
    total_size = num;
    actual_size = 0;
  }

  bool addA(A* x)
  {
    if(actual_size >= total_size) return false;
    a[actual_size++] = x;
    return true;
  }


  ~B()
  {
  /*  for(int i=0; i<actual_size; i++)
    {
      delete a[i];
     }*/
    delete[] a;
  }

};

void foo()
{
  B* b = new B(10);
  A* a1= new A(1);
  A* a2 = new A(2);
  b->addA(a1);
  b->addA(a2);
  delete b;
  delete a1;
  delete a2;
  // no leak
}