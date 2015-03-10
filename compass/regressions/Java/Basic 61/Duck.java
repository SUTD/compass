
import verify.*;

 
class Tester
{
  void test()
  {
    String abc = "abc";
    String x = abc;
    Compass.static_assert(x == "abc");
  }
}