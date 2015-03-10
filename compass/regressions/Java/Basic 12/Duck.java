
import verify.*;




class Tester {

  int test(int a, int b, int c)
  {
      int x;
      if(a< b || b<c) {
	x= 1;
      }
      else x = 2;
      return x;

  }

  void caller()
  {
    int c = test(1, 2, 1);
    Compass.static_assert(c == 1);
    
  }
  
}

