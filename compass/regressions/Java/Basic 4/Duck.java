
import verify.*;

class Foo
 {
    Baz a;
    int x;
 };

  class Bar {
      int b;
      Foo f;
  };

class Baz
{
  int c;
  int d;
};

 
  
class Tester {
  public Bar duck(int a, int c)
  {
    
   
    Bar bar = new Bar();
    Foo foo = new Foo();
    Baz baz = new Baz();
    baz.c = 1;
    baz.d = 2;
    foo.a = baz;
    bar.f = foo;
    bar.f.a.c = 4;
    foo.a.d = 6;
    Compass.static_assert(baz.c == 4);
    Compass.static_assert(bar.f.a.d == 6);
    return bar; 
  }



};
