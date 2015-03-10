
import verify.*;



class Tester {

  int test(int a, int b)
  {
      int x;
      if(a<b) {
	x= 1;
      }
      else x = 2;
      return x;

  }

  void caller()
  {
    int c = test(3, 7);
    int d = test(5, 1);
    Compass.static_assert(d > c);
  }
  
}

