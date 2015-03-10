using namespace spec;

void foo()
{
  vector<int> a;
  a.push_back(1);
  a.push_back(2);


 vector<int>::iterator it = a.begin();
 int x = *it;
 static_assert(x == 1);
 it++;
 x = *it;
 static_assert(x == 2);
 it++;
 bool b = it == a.end();
 static_assert(b);

  
}