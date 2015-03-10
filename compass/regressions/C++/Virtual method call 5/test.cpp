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

void test(int flag)
{
  Animal* a = 0; 
  if(flag) a = new Duck();
  else a = new Bunny();
  int food = a->eat();
  if(flag) static_assert(food == DUCK); 
  else static_assert(food == BUNNY);
}