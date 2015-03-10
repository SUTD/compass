enum animal_type
{
  ANIMAL,
  DUCK,
  BUNNY
};

class Animal
{
int weight;
public:
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
  Duck* d = new Duck();
  int b= bar(d);
  static_assert(b == DUCK);
}