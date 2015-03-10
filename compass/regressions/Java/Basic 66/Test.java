import verify.*;

class Bar
{
  int x;
  int y;
}

class Tester
{
    void test()
    {
      Bar b = new Bar();
      Compass.static_assert(b.x == 0);
    }
}