
using namespace spec;


void bar()
{
  queue<int> q;
  q.push(34);
  q.push(23);
  
  int x = q.front();
  static_assert(x == 34);
  
}