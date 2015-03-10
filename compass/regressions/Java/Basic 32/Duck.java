
import verify.*;



class Foo
 {
    int x;
 };

  class Bar {
      Foo f;
  };

 
class Tester {  
  public Bar duck(int a, int c)
  {
    
    Bar b = new Bar();
    b.f = new Foo(); //147
    b.f.x = 3;
 
    return b; 
  }

  public void duck2(int c)
  {
    Bar b = duck(0, 3);
    Compass.static_assert(b.f.x==3);
  }

}