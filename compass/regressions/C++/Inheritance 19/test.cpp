
enum animal_type {
  SQUIRREL,
  RABBIT,
  BIRD
};

class Food {
public:
   virtual bool is_low_calorie_food() = 0;
   virtual bool is_high_calorie_food() = 0;

};

class Carrot: public Food
{
public:
  Carrot() 
  {
    
  }

  virtual int get_num_calories()
  {
    return 5;
  }

  virtual bool is_low_calorie_food()
  {
    return true;
  }

  virtual bool is_high_calorie_food()
  {
    return false;
  }

};

class Apple: public Food
{
  public:
  
  virtual int get_num_calories()
  {
    return 20;
  }

   virtual bool is_low_calorie_food()
  {
    return true;
  }

  virtual bool is_high_calorie_food()
  {
    return false;
  }

};

class Nut: public Food
{
public:
  Nut() {};
  virtual int get_num_calories()
  {
    return 50;
  }

  virtual bool is_low_calorie_food()
  {
    return false;
  }

  virtual bool is_high_calorie_food()
  {
    return true;
  }
};



class Animal
{

protected:
  int length;
  int weight;
  animal_type at;
 

public:

  

  Animal(int length, int weight): length(length), weight(weight) {};
  int get_length() {return length;};
  int get_weight() {return weight;};
  int get_animal_type() {return at;};
  virtual Food* get_favorite_food() =0;
 virtual bool is_mammal() = 0;

  
};

class Rabbit: public Animal
{
Food* favorite_food;
    public:
    Rabbit(int length, int weight, Food* favorite_food): Animal(length, weight)
    {
      this->at = RABBIT;
      this->favorite_food = favorite_food;
    }

    virtual Food* get_favorite_food()
    {
      return favorite_food;
    }

    virtual bool is_mammal()
    {
      return true;
    }
};

class Squirrel: public Animal
{
Food* favorite_food;
    public:
    Squirrel(int length, int weight, Food* favorite_food): Animal(length, weight)
    {
      this->at = SQUIRREL;
      this->favorite_food = favorite_food;
    }

    virtual Food* get_favorite_food()
    {
      return favorite_food;
    }

    virtual bool is_mammal()
    {
      return true;
    }
};

class Bird: public Animal
{
Food* favorite_food;
    public:
    Bird(int length, int weight, Food* favorite_food): Animal(length, weight)
    {
      this->at = BIRD;
      this->favorite_food = favorite_food;
    }

    virtual Food* get_favorite_food()
    {
      return favorite_food;
    }

    virtual bool is_mammal()
    {
      return false;
    }
};




void need_low_calorie_foods(Animal** pets, int num_pets)
{
  
  for(int i=0; i<num_pets; i++)
  {
    Animal* a = pets[i];
    Food* f = a->get_favorite_food();
    static_assert(f->is_low_calorie_food());
  }

}


void test()
{
  Food* c = new Carrot();
  Animal* pet1 = new Rabbit(5, 10, c);
  Food* n = new Nut();
  Animal* pet2 = new Squirrel(4, 8, n);
  Food* apple = new Apple();
  Animal* pet3 = new Bird(2,3, apple);
  Animal* pet4 = new Rabbit(3, 4, c);
  

  Animal* my_pets[3];
  my_pets[0] = pet1;
  my_pets[1] = pet3;
  my_pets[2] = pet4;

  need_low_calorie_foods(my_pets, 3);

  
}