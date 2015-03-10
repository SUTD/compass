
import verify.*;


class Foo
{
  int y;
  Bar b;
  
  
};

class Bar
{
  int x;
  Foo f;

};





class Tester {

 void test()
 {
    Foo f = new Foo();
    f.b = new Bar();
    f.b.f = null;
    Compass.static_assert(f.b.f == null);
 }
  
}
