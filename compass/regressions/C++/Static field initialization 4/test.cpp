

class Foo
{
  public:
  static int a;
  Foo()
  {
    a++;
  }
};


int Foo::a = 0;



int test()
{
  
  static_assert(Foo::a == 0); //correct
  Foo f;
  static_assert(Foo::a == 1); //correct
  Foo f3;
  static_assert(Foo::a == 1); //should fail
  
}