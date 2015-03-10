
import verify.*;

class Foo
{
  int x;
  int y;
};

 class Tester {

void test(int a)
{
    Foo f= new Foo();
    if(a>0) {
      f.y = a;
    }
    else f.y = -a;
    Compass.static_assert(f.y>=0);
}

}
