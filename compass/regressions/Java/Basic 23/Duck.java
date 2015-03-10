
import verify.*;




class Tester {

class Foo 
{
  int y;

};

 void test()
 {
    Foo f = new Foo();
    f.y = 2;
    Compass.static_assert(f.y == 2);
 }
  
}