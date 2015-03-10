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
};

void foo(A* a1, A* a2)
{
  delete a1->b;
  delete a2->b;

}

void bar()
{
   B* b = new B();
   A* a1  = new A(b);
  foo(a1, a1); //error

}