
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
   Bar b= new Bar();
   b.a  = 2;
   b.b = 3;
   Foo  f = b;
   Compass.static_assert(f.a==2);
}

}
