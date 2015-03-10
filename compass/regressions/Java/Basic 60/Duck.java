
import verify.*;

 
 class Member<T> {
  T id;


 }
 
class Foo
{
  int x;
};

 class Tester {
  void test() {
    Foo f = new Foo();

    Member<Foo> m = new Member<Foo>();
    m.id = f;
    Foo a = m.id;
    Compass.static_assert(a == f);
  } 
 
 }