
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
   Bar b = new Bar();
   b.a = 2;
   Foo f = (Foo) b;
   Compass.static_assert(f.a==2);
}

}