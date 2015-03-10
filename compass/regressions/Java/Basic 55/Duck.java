
import verify.*;


class Bar
{
  int x; 
  int y;
}

class Foo
{
  static Bar b;
  static void init_bar() {b = new Bar();} ;
}



class Tester 
{

    void test()
    {
      Foo.init_bar();
      Foo.b.x = 2;
      Foo.b.y = 4;
      Compass.static_assert(Foo.b.x == 2);
    }

}