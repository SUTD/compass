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
  Animal a;
  Animal b;
  b.weight = 77;
  a = b;
  static_assert(a.weight == 1); //should fail

}