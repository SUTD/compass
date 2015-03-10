
import verify.*;




class Foo
{
  Bar b;
  Foo()
  {
    b = new Bar();
  }
  
};

class Bar
{
  int x;

};




class Tester {

 void test()
 {
    Foo f = new Foo();
    f.b.x = 2;
    Compass.static_assert(f.b.x == 2);
 }
  
}