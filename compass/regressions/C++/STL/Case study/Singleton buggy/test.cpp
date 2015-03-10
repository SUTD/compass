
/*
 * Buggy version of Singleton example.
 * The Singleton class implements a get_shared method for points, 
 * such that the result of get_shared should be a point with the same
 * x and y coordinates if such a point exists, otherwise, it
 * is the same point. 
 * The check_correctness function in Singleton asserts the functional 
 * correctness of the get_shared method.

 * Bug: The implementation of operator==() for points is wrong!
 */

using namespace spec;


class Point
{
  private:
    int x;
    int y;
  public:
    Point()
    {
      x = 0;
      y = 0;
    };

    Point(int x, int y)
    {
    this->x = x;
    this->y = y;
    }

    int get_x()
    {
      return x;
    };
    int get_y()
    {
      return y;
    }

   bool operator==(const Point& other)
   {
      return x == other.x;
   }
};


class Singleton
{
  private:
    set<Point*> points;
  public:
    Point* get_shared(Point* p)
    {
      Point* existing = NULL;
      set<Point*>::iterator it = points.begin();
      for(; it!= points.end(); it++)
      {
    Point* cur = *it;
    if(*cur == *p) {
      existing = cur;
      break;
      }
    }

     if(existing == NULL) {
      points.insert(p);
      return p;
     }

     else {
      delete p;
      return existing;
     }

    };


  void check_correctness(int x, int y)
  {
      set<Point*> b;
      Point* p = new Point(x, y);
      Point* shared_p = get_shared(p);
      if(shared_p != p) {
	static_assert(shared_p->get_x() == x);
	static_assert(shared_p->get_y() == y); //should fail!
      }
  }

};