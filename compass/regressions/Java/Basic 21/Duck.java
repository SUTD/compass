
import verify.*;


class Foo
{
  int x;
  void setX() {this.x = 11;};
};


class Bar extends Foo
{
  void setX() {this.x = 22;}

}

class Baz extends Bar
{
  void setX() {this.x = 33;}
}

class Tester {

 void test(boolean flag)
 {
    Foo f = new Bar();
    f.setX();
    f = new Baz();
    f.setX();
    Compass.static_assert(f.x == 33);
    
 }
  
}


