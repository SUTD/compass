enum animal_type
{
   ANIMAL,
   BIRD,
   DUCK
};

class Animal
{
  public:
  virtual int get_weight() {return 1;};
  virtual animal_type get_type() {return ANIMAL;};
};

class Bird:public Animal
{
  public:
  virtual int get_weight() {return 5;};
 virtual animal_type get_type() {return BIRD;};
};

class Duck: public Bird
{
public:
 virtual animal_type get_type() {return DUCK;};
};

int test(Animal* a)
{
  int res = a->get_weight();
  if(res == 5) static_assert(a->get_type() != ANIMAL); // correct
  return res;
 
}