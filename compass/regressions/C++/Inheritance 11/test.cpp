
enum animal_type {
  GENERIC_ANIMAL,
  SQUIRREL,
  RABBIT,
  BIRD
};

class Food {
  
  public:
    virtual int get_num_calories() = 0;
    static bool is_low_calorie_food(Food* f) {return f->get_num_calories() < 30;};
    static bool is_high_calorie_food(Food* f)  {return f->get_num_calories() >= 30;};

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
};

class Nut: public Food
{
public:
  Nut() {};
  virtual int get_num_calories()
  {
    return 50;
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







void test()
{
  Food* c = new Carrot();
  Animal* pet1 = new Rabbit(5, 10, c);

  static_assert(pet1->get_animal_type() == RABBIT);
  static_assert(pet1->get_favorite_food()->get_num_calories() == 5);
  static_assert(Food::is_low_calorie_food(pet1->get_favorite_food()));

   Food* n = new Nut();
  Animal* pet2 = new Squirrel(4, 8, n);
  static_assert(Food::is_high_calorie_food(pet2->get_favorite_food()));

  Animal* pet3 = new Bird(2,3, n);
  

  int num_pets = 3;
  Animal* my_pets[3];
  my_pets[0] = pet1;
  my_pets[1] = pet2;
  my_pets[2] = pet3;

  for(int i=0; i<num_pets; i++)
  {
    static_assert(my_pets[i]->is_mammal()); //should fail!
  }


  
}