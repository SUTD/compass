
import verify.*;


class Foo
{
  int y;
  Bar b;
  
  
};

class Bar
{
  int z;
  Cat c;

};


class Cat
{
  int x;
};



class Tester {

 void test()
 {
    Foo f = new Foo();
    f.b = new Bar();
    f.b.c = new Cat();
    f.b.c.x = 2;
    f.y = 3;
    f.b.z = 4;
    Compass.static_assert(f.b.c.x == 2);
    Compass.static_assert(f.y == 3);
    Compass.static_assert(f.b.z == 4);
 }
  
}
