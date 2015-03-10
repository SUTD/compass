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
  
  bool operator==(const Point& other) const
  {
    return other.x == x && other.y == y;
  }

};

void test()
{
  Point* p1 = new Point(1, 2);
  Point* p2 = new Point(1, 2);
  static_assert(*p1 == *p2); 
}