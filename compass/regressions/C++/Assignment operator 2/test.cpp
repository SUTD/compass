class Foo
{
public:
int x;
Foo(){};
virtual int get(){return x;};

};

void test()
{
  Foo f;
  f.x = 55;

  Foo f2 = f;
  f2.x = 7;
  static_assert(f2.x == 55); //should fail
}