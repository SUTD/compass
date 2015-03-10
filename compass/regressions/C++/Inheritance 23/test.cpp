
enum animal_type {
  GENERIC_ANIMAL,
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




class Zoo
{
  private:
  Animal** animals;
  int num_animals;

  public:

  Zoo()
  {
    animals = new Animal*[50];
    num_animals = 0;
  }
  
  void add_animal(Animal* a)
  {
    static_assert(num_animals < 50);
    animals[num_animals++] = a;
  }
  int get_num_animals() 
  {
    return num_animals;
  }
  Animal* operator[](int i)
  {
    return animals[i];
  }

};


void test()
{
  Food* c = new Carrot();
  Animal* rabbit = new Rabbit(5, 10, c);
  Food* n = new Nut();
  Animal* squirrel = new Squirrel(4, 8, n);
  Food* apple = new Apple();
  Animal* bird = new Bird(2,3, n);
  Animal* rabbit2 = new Rabbit(3, 4, c);
  

  Zoo z;
  z.add_animal(rabbit);
  z.add_animal(bird);
  z.add_animal(squirrel);
  
  int num_animals = z.get_num_animals();
  static_assert(num_animals == 2); //should fail


  
}