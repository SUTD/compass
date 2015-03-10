
import verify.*;


class Foo
{
public int x;
void setX() 
{
    x = 2;
}
};

class BigFoo extends Foo
{
};


class Tester 
{

    void test()
    {
      Foo f = new BigFoo();
      f.setX();
      int y = f.x;
      Compass.static_assert(y == 2);
    }
};

