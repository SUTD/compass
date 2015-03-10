
import verify.*;


class Tester
{
  void test(int n)
  {
    int[] a = new int[10];


 for(int i=0; i<10; i++) {
      a[i] = 77;
    }
    Compass.static_assert(a[1] == 77);

  }


}
