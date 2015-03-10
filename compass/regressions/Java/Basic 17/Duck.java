
import verify.*;



class Foo
{
  int x;
  int y;
  void setX(int x) {this.x = x;}
  void setY(int y) {this.y = y;}
}

class Tester {

 void test()
 {
    Foo f = new Foo();
    f.setX(2);
    f.setY(3);
    Compass.static_assert(f.x == 2);
    Compass.static_assert(f.y == 3);
 }
  
}


