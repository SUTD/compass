
class Duck
{
public:
  int length;
  int weight;
};

void foo()
{
  
  Duck d;
  d.length = 20;
  d.weight = 5;
  int a = d.length;
  static_assert(a == 20);
  static_assert(d.weight == 5);
  
}