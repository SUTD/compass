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
  Duck d;
  Bunny b;
  Animal* a;
  if(flag) a= &d;
  else a = &b;
  int res= bar(a);
  if(!flag) static_assert(res == DUCK); //should fail
}