
import verify.*;


class Bar
{
  int y;
};


class Foo
{
  int x;
  Bar b;
}

class Tester
{
  void test()
  {
    Foo f1 = new Foo();
    f1.x = 3;
    Bar b = new Bar();
    b.y = 4;
    f1.b = b;
    f1.b.y = 6;
    Compass.static_assert(b.y == 6);

  }
}
