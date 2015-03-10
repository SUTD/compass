
using namespace spec;


void bar(int size)
{

  assume(size >= 1);

  stack<int> s;
  queue<int> q;

  for(int i=0; i<size; i++)
  {
    s.push(i);
    q.push(i);
  }

  int stack_first = s.top();
  int queue_first = q.front();

  static_assert(stack_first == size-1);
  static_assert(queue_first == 0);
  
  
  
}