
import verify.*;

  class Foo
   {
      int x;
      int getX() { return x;}
      void setX(int a) { x= a;}
   }

class Tester {

 void test()
 {
    Foo f = new Foo();
    f.setX(2);
    int a = f.getX();
    Compass.static_assert(a==2);
}

};
