
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
  static_assert(Foo::x == 1);
  
}