enum animal_type
{
  ANIMAL,
  DUCK,
  BUNNY
};

class Animal
{
public:
int weight;

Animal()
{
  eat();
}

public:
 virtual int eat() { return ANIMAL;};
};


class Duck: public Animal
{
public:
 Duck()
 {
 };

 virtual int eat() { return DUCK;};

};

class Bunny:public Animal
{
public:
 Bunny()
 {
 };

 virtual int eat() { return BUNNY; };
};

int bar(Animal* a)
{
  return a->eat();
}

void foo(int flag)
{
  Animal* zoo[3];
  zoo[0] = new Bunny();
  zoo[1] = new Duck();
  zoo[2] = new Animal();

  int r1 = zoo[0]->eat();
  static_assert(r1 == BUNNY);

  int r2 = zoo[1]->eat();
  static_assert(r2 == DUCK);
  
  int r3 = zoo[2]->eat();
  static_assert(r3 == ANIMAL);


}