
int x;

class Foo
{
  public:
    static int  x;
    Foo()
    {
    };
};


void test()
{
  Foo f;
  Foo::x = 1;
  Foo::x = 77;
  static_assert(Foo::x == 77); // correct
  
}