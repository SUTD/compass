
import verify.*;



class Tester {

  void test()
  {
    int i;
    int t;
    for(i=0; i<10; t=i+1, i=t) {

    }
    Compass.static_assert(i>=10);
  

  }
  
}


