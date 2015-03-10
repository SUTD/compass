
import verify.*;



class Foo
{
  int y;
  Bar b;
  
  
};

class Bar
{
  int x;

};




class Tester {

 void test()
 {
    Foo f = new Foo();
    f.b = new Bar();
    f.b.x = 2;
    f.y = 3;
    Compass.static_assert(f.b.x == 2);
    Compass.static_assert(f.y == 3);
 }
  
}
