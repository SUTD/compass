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
}

public:
 int get_animal_type() {return get_type_internal(); };
 virtual int get_type_internal() { return ANIMAL;};
};


class Duck: public Animal
{
public:
 Duck()
 {
 };

 virtual int get_type_internal() { return DUCK;};

};

class Bunny:public Animal
{
public:
 Bunny()
 {
 };

 virtual int get_type_internal() { return BUNNY; };
};

int bar(Animal* a)
{
  return a->get_animal_type();
}

void foo(int flag)
{
  Duck d;
  int b= bar(&d);
  static_assert(b == ANIMAL); //should fail
}