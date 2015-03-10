using namespace spec;

void foo()
{
  map<int, vector<int> > m;
  vector<int> v;
  v.push_back(1);
  v.push_back(2);
  m[9]=v;


 vector<int> v2;
  v2.push_back(6);
  v2.push_back(7);
  m[3]=v2;
  
  int xx = m[9][0];
  int yy = m[9][1];
  static_assert(xx == 1);
  static_assert(yy == 2);

  static_assert(xx == 3); //should fail
  static_assert(yy == 4); //should fail


 int a = m[3][0];
 int b = m[3][1];


 static_assert(a == 6);
 static_assert(b == 7);

 static_assert(a == 1); //should fail
 static_assert(b == 2); //should fail


  
}