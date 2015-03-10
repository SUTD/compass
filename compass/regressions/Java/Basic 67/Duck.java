
import verify.*;


class Polygon
{
public int x;
};

class Rectangle extends Polygon
{
};

class Triangle extends Polygon
{
Triangle(int x, int y){
}
};

class Tester 
{

    void test()
    {
      Polygon r;
      int x = 0;
      int k = 0;
      if(x==1)
	r = new Triangle(x, k);
      k = 5;
      Compass.static_assert(k == 5);
    }
};