
import verify.*;


 class Foo
{
  int a;
};

class Bar extends Foo
{
  int b;
};

 class Tester {

void test()
{
   Foo f = new Bar();
   f.a  = 2;
   Bar b = (Bar) f;
   Compass.static_assert(b.a==2);
}

}