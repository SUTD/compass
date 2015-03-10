using namespace spec;


/*
 * We collect those  keys in map m that have non-null values 
 * in set keys and check functional correctness.
*/

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

void test()
{

  map<string, Point*> m;
  m["aa"] = new Point(8, 8);
  m["fg"] = NULL;
  m["zzz"] = new Point(2, 23);;
  set<string> keys;

  map<string,Point*>::iterator it = m.begin();
  for(; it!= m.end(); it++)
  {
    string s = it->first;
    Point* p = it->second;
    if(p!=NULL)
      keys.insert(s);
  }

  static_assert(keys.count("aa") > 0);
  static_assert(keys.count("fg") == 0);
  static_assert(keys.count("zzz") > 0);

}