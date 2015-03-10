
import verify.*;



class Tester {

  void test()
  {
    int a =1;
    int b=2;
    int max = (a > b) ? a : b;
    Compass.static_assert(max == 2);

  }
  
}