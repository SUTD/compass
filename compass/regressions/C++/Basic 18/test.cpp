

class Animal
{
public:
  int length;

public:
  int get_length()
  {
    return length;
  }

};



class Duck: public Animal
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
  int weight;
};


void foo()
{
  Duck d(20, 5);

  d.eat();
  d.fly();

   static_assert(d.get_weight() == 5);
   static_assert(d.get_length() == 20);

}