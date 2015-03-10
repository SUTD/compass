
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
};


void test(int size)
{

  vector<Point*> v;
  for(int i=0; i < size; i++)
  {
      v.push_back(new Point());
  }

  Point* p = new Point(2, 3);

  v[0] = p;
  v[1] = p;

  vector<Point*>::iterator it = v.begin();
  for(; it!= v.end(); it++)
  {
    Point* p1 = *it;
    vector<Point*>::iterator it2 = it;
    it2++;	
    for(; it2!= v.end(); it2++)
    {
      Point* p2 = *it2;
      static_assert(p1 != p2);
    }

  }
 


}