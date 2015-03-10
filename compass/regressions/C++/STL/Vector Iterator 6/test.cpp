using namespace spec;

void foo()
{
  vector<int> a;
  a.push_back(1);
  a.push_back(2);


 vector<int>::iterator it = a.begin();
 int x = a.size();
 static_assert(x == 2);

 for(; it != a.end(); it++)
 {
    x = *it;
    static_assert(x >1); //should fail
 }

  
}