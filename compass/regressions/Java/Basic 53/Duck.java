
import verify.*;


class Foo
{
  static int x = 0;
}



class Tester 
{


    void test()
    {
      Foo.x = 2;
      Compass.static_assert(Foo.x == 2);
    }

}