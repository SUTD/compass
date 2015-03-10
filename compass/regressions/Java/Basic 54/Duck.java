
import verify.*;


class Foo
{
  static int x = 0;
  static void setX(int y) {x = y;} ;
  static int getX() {return x; }
}



class Tester 
{

    void test()
    {
      Foo.setX(44);
      int y = Foo.getX();
      Compass.static_assert(y == 44);
    }

}