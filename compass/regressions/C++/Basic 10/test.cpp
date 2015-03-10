
class Duck
{
public:
  Duck(int length, int weight)
  {
    this->length = length;
    this->weight = weight;
  };
  void eat()
  {
    weight++;
  }
  ~Duck()
  {
     static_assert(length == 20);
     static_assert(weight == 6);
  }

  int length;
  int weight;
};


void foo()
{
  Duck* d = new Duck(20, 5);
  int a = d->length;
  static_assert(a == 20);
  static_assert(d->weight == 5);

  d->eat();
  delete d;
  
}