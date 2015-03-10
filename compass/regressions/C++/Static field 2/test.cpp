
class Foo
{
  public:
    static const int  x =1;
    Foo()
    {
    };
};

void test()
{
  Foo f;
  static_assert(Foo::x == 2); // should fail
  
}