
class Animal
{
  int length;
  int weight;

public:

  Animal(int length, int weight): length(length), weight(weight) {};
  int get_length() {return length;};
  int get_weight() {return weight;};
  
};

void test()
{
  Animal* a = new Animal(2, 3);
  static_assert(a->get_length() == 2);
  static_assert(a->get_weight() == 3);
}