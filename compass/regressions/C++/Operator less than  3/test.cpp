class Point
{
  private:
  int x;
  int y;
  
  public:
  Point(int x, int y)
  {
    this->x = x;
    this->y = y;
  }
  
  bool operator<(const Point& other) const
  {
    if(x > other.x) return false;
    if(x< other.x) return true;
    return (y < other.y);
  }

};

void test()
{
  Point p1(0, 1);
  Point p2(0, 1);
  static_assert(p2 < p1); //should fail
  
}