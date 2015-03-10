
import verify.*;


class Foo
{
  int x;
  int y;
  Foo()
  {
  }
}

class Tester {

 void test()
 {
    Foo f = new Foo();
    f.x = 2;
    f.y = 3;
    Compass.static_assert(f.x == 2);
    Compass.static_assert(f.y == 3);
 }
  
}

