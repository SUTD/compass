

class Foo
{
  public:
  static int a;
};


int Foo::a = 44;



int test()
{
  
  static_assert(Foo::a == 44); 
  
}