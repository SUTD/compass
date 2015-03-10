
class Duck
{
public:
  Duck(int length, int weight)
  {
    this->length = length;
    this->weight = weight;
  };
  ~Duck()
  {
  //both should fail
  static_assert(length == 1);
  static_assert(weight == 2);
  }

  int length;
  int weight;
};


void foo()
{
  Duck* d = new Duck(20, 5);
  delete d;
  
}