
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

  map<int, Point*> m;
  m[55] = new Point(55, 0);
  m[23] = new Point(34, 23);
  m[4] = new Point(8, 9);
  m[12] = new Point(45, 5);
  m[45] = NULL;

  map<int, Point*>::iterator it = m.begin();
  for(; it!= m.end(); it++)
  {
    Point* p = it->second;
    static_assert(p != NULL);
  }
}