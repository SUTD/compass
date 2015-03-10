#define NULL 0

class Animal
{
  public:
  virtual bool is_mammal() = 0;
};

class Bird:public Animal
{
  public:
  virtual bool is_mammal() {return false;};
};

class Rabbit:public Animal
{
virtual bool is_mammal() {return true;};
};

Animal* find_mammal(Animal** animals, int num)
{
  

  Animal* m = NULL;
  for(int i=0; i<num; i++) {
    Animal* a = animals[i];
    if(a->is_mammal()) {
	m = a;
	break;
    }
  }
  
  if(m!=NULL) static_assert(m->is_mammal());
  return m;
}