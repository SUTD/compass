
import verify.*;



 class Tester {

void test()
{
  int x = 2;
  int y;
  
  switch (x)
  {
    case 1: y=1; break;
    case 2: y=2; break;
    case 3: y=3; break;
    default: y=4;
  }

  Compass.static_assert(y==2);

}

}
