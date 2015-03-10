#include <stdlib.h>

void specify_checks()
{
  check_null();
}


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
  Animal* a = NULL;
  if(flag) a = new Duck();
  int food = a->eat(); // null dereference
}