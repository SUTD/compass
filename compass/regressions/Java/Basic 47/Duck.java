
import verify.*;


class Bar
{
  int y;
  void setY(int y) {this.y = y;}
  int getY() {return y;}
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
  void baz(Foo f)
  {
    f.getB().setY(77);
  }

  void test()
  {
    Foo f1 = new Foo();
    f1.x = 3;
    Bar b = new Bar();
    b.y = 4;
    f1.setB(b);
    baz(f1);
    Compass.static_assert(b.getY() == 77);

  }
}
