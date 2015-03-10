
using namespace spec;


void bar()
{
  stack<int> s;
  s.push(7);
  s.push(8);
  s.push(45);
  int x = s.top();
  static_assert(x == 45);
  s.pop();
  int y = s.top();
  static_assert(y == 7); //should fail
  
}