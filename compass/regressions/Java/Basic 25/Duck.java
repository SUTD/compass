
import verify.*;





class Bar
{
  int x;

};

class Foo
{
  Bar b;
};




class Tester {

 void test()
 {
    Foo f = new Foo();
    f.b = new Bar();
    f.b.x = 2;
    Compass.static_assert(f.b.x == 2);
 }
  
}