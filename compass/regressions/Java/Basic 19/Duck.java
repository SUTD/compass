
import verify.*;



class Bar
{
  int x;
  void setX() {this.x = 11;};
};


class Foo extends Bar
{
  void setX() {this.x = 22;}

}

class Tester {

 void test()
 {
    Bar b = new Foo();
    b.setX();
    Compass.static_assert(b.x == 22);
 }
  
}


