
import verify.*;


class Foo
{
  int x;
  int getX() {return x;}
  void setX() {this.x = 0;}
  
};

class Bar extends Foo
{
  void setX() {this.x = 11;}

};

class Baz extends Foo
{
 void setX() {this.x = 22;}
};





class Tester {

 Foo test1()
 {
  Foo f= new Bar();
  f.setX();
  return f;
 }

 void test2()
 {
    Foo f = test1();
    int a = f.getX();
    Compass.static_assert(a == 11);
    
 }
  
}
