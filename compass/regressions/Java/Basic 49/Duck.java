
import verify.*;


class Baz 
{
  int x;
  int y;
}


class Foo
{
  int a;
  Baz baz;
};

class Bar extends Foo
{
  int b;
};

 class Tester {

void test()
{
   Bar b = new Bar();
   b.a = 2;
   b.baz = new Baz();
   b.baz.x = 43;
   b.baz.y = 77;
   Foo f = (Foo) b;
   Compass.static_assert(f.baz.y==77);
}

}