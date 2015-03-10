
import verify.*;




class Tester {
int x;
 void setX(int x){this.x = x;}
 int getX() {return x; }

class Foo 
{
  int y;
  
  void setY()
  {
    this.y = getX();
  }
  
  int getY()
  {
    return y;
  }

};

 void test()
 {
    setX(2);
    Foo f = new Foo();
    f.setY();
    int a = f.getY();
    Compass.static_assert(a == 2);
 }
  
}