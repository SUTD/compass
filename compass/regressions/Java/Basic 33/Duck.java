
import verify.*;




class Foo
 {

    public Baz a;
    private int x;

    public Baz getA(){return a;};
    public int getX(){ return x;};
    public void setA(Baz b){a = b;};
    public void setX(int x){ this.x = x;};
 };


class Baz
{
  int c;
  int d;
};

 
  
class Tester {
  public void duck(int a, int c)
  {
      Foo foo = new Foo();
      Baz baz = new Baz();
      baz.d = 4;

      foo.setA(baz);
      Baz baz2 = foo.getA();
      Compass.static_assert(baz == baz2);

  /*    Foo foo = new Foo();
      foo.setX(4);
      Compass.static_assert(foo.getX() == 4);*/
    
  }
}