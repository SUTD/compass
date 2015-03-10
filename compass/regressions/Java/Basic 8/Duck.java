
import verify.*;




class Tester {

  void test()
  {
    int i = 0;
    int a = i+2;
    i=a;
    Compass.static_assert(i==2);
  }
  
}

