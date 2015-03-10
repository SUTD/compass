namespace campus
{
  class Duck
  {
public:
  int length;
  int weight;
    Duck(int length, int weight)
    {
      this->length = length;
      this->weight = weight;
    }
  void eat()
  {
    weight++;
  }

  void fly()
  {
    weight-=2;
  }
  };
}



void static_assert(int i);

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

  void fly()
  {
    weight--;
  }
  ~Duck()
  {

  }

  int length;
  int weight;
};


void foo()
{
  Duck d(20, 5);
  int a = d.length;

  d.eat();
  d.fly();

   static_assert(d.weight == 5);

  campus::Duck cd(20, 5);
  cd.eat();
  cd.fly();
  //should fail
  static_assert(cd.weight == 5);
}