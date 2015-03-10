

using namespace spec;





void foo(int k)
{
  /*map<int, vector<int> > m;
  vector<int> v;
  v.push_back(1);
  v.push_back(2);
  m[87] = v;
  
  vector<int>& y = m[87];
  int x = y[0];

  static_assert(x ==1);
 // static_assert(m[87][1] ==2);
*/


}






template<class T1, class T2>
  struct pp
  {
    T1 first;
    T2 second;

    pp()
    : first(), second() { }


    pp(const T1& a, const T2& b)
    : first(a), second(b)
  {

  }

  };


struct foo
{
  int x; 
  int y;
  foo(int x, int y) {this->x = x; this->y = y;}

  foo(const foo & other)
  {
    x = other.x;
    y = other.y;
  }
};

void ffff()
{
  vector<int> g;
  g.push_back(4);
  g.push_back(6);
  pp<int, vector<int> > x(3,g);
  static_assert(x.first == 3);
  static_assert(x.second[0] == 4);
  static_assert(x.second[1] == 6);

 

}