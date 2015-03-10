namespace campus
{
  class Duck
  {
private:
  int length;
  int weight;
public:
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

  int get_weight()
  {
    return weight;
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
  int get_weight()
  {
    return weight;
  }

private:
  int length;
  int weight;
};


void foo()
{
  Duck d(20, 5);

  d.eat();
  d.fly();

   static_assert(d.get_weight() == 5);

  campus::Duck cd(20, 5);
  cd.eat();
  cd.fly();
  
  static_assert(cd.get_weight() == 4);
}