void specify_checks()
{
  check_cast();
};

class A
{
  public:
  A() {};
  virtual int foo() {return 1;}

};

class B: public A
{
  public:
  B(){};
  virtual int foo() {return 2;}
};

void test(B** b, int num)
{
  for(int i=0; i<num; i++)
  {
    b[i]->foo();
  }
}

void bar()
{
  B* bs[3];
  bs[0] = new B();
  bs[1] = (B*) new B();
  bs[2] = new B();

  test(bs, 3); //ok
  

}