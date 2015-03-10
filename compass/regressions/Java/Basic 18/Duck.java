
import verify.*;



class Foo
{
  int x;
  int y;
  void setX(int x) {this.x = x;}
  void setY(int y) {this.y = y;}
  int getX(){return x;}
  int getY(){return y;}
}

class Tester {

 void test()
 {
    Foo f = new Foo();
    f.setX(2);
    f.setY(3);
    int x = f.getX();
    int y = f.getY();
    Compass.static_assert(x == 2);
    Compass.static_assert(y == 3);
 }
  
}


