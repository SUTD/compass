
import verify.*;



class Foo
{
  int x;
}

class Tester
{
  void test()
  {
    Foo f1 = new Foo();
    f1.x = 3;
    Foo f2 = f1;
    f2.x = 4;
    Compass.static_assert(f1.x == 4);

  }
}
