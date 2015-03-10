
import verify.*;


class Tester {

  int test(int a, int b)
  {
      int x;
      if(a<=b) {
	x= 1;
      }
      else x = 2;
      return x;

  }

  void caller(int x, int y)
  {
    int c = test(x, y);
    if(x > y) {
      Compass.static_assert(c > 1);
    }
  }
  
}

