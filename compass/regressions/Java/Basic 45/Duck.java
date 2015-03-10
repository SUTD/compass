
import verify.*;


class Bar
{
  int y;
  void setY(int y) {this.y = y;}
};


class Foo
{
  int x;
  Bar b;

  void setB(Bar b){ this.b=b;}
  Bar getB() {return b;}
}

class Tester
{
  void test()
  {
    Foo f1 = new Foo();
    f1.x = 3;
    Bar b = new Bar();
    b.y = 4;
    f1.setB(b);
    Bar b2 = f1.getB();
    b2.y = 6;
    Compass.static_assert(b.y == 6);

  }
}
