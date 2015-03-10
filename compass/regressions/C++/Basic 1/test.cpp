
struct Duck
{
  int length;
  int weight;
};

void foo()
{
  
  Duck d;
  d.length = 2;
  int a = d.length;
  static_assert(a == 2);
  
}