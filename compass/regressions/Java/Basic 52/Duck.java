
import verify.*;


class Tester 
{

   public static int x;

    void test()
    {
      Tester.x = 2;
      Compass.static_assert(Tester.x == 2);
    }

}