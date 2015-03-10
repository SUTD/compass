import verify.*;

class Point2D{
  int x;
  int y;

  Point2D(int x, int y) {
    this.x = x;
    this.y = y;
  }

  void set(int x, int y)
  {
    this.x = x;
    this.y = y;
  }


}


class Point3D extends Point2D
{
  int z;
  Point3D(int x, int y, int z)
  {
    super(x,y);
    this.z = z;
   }

  void set(int x, int y, int z)
  {
    super.set(x,y);
    this.z = z;
   }
}

class Tester
{
  void test()
  {
     Point3D p = new Point3D(1,2,3);

     Compass.static_assert(p.x == 1);
     Compass.static_assert(p.y ==2);
     Compass.static_assert(p.z == 3);

    p.set(4,5,6);
     Compass.static_assert(p.x == 4);
     Compass.static_assert(p.y ==5);
     Compass.static_assert(p.z == 6);


  }
}