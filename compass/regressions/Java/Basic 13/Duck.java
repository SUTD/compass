
import verify.*;





class Tester {

  void test()
  {
    int i=0;
    while(i<10) {int t= i+1; i =t; }
    Compass.static_assert(i>=10);
  

  }
  
}


