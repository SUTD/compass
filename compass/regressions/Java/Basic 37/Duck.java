
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
    Foo f;
    if(flag) f = new Bar();
    else f = new Baz();
    f.setX();
    if(!flag) {
      Compass.static_assert(f.x == 33);
    }
    
 }
  
}