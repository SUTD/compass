
import verify.*;




class Tester {
int x;

class Foo 
{
  int y;
  
  void setY()
  {
    this.y = x;
  }

};

 void test()
 {
    x = 2;
    Foo f = new Foo();
    f.setY();
    Compass.static_assert(f.y == 2);
 }
  
}