
import verify.*;




class Foo
{
  int x;
  int y;
  Foo(int x, int y)
  {
    this.x = x;
    this.y = y;
  }
}

class Tester {

 void test()
 {
    Foo f = new Foo(2, 3);
    Compass.static_assert(f.x == 2);
    Compass.static_assert(f.y == 3);
 }
  
}