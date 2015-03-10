class Food
{
  public:
  virtual int get_food() {return 1;};

};

class Broccoli:public Food{
  public:
    virtual int get_food() {return 2;};
};

class Animal
{
  public:
    int x;
    Food f;
    
  


};

void test(Animal* a)
{
  int f = a->f.get_food();
  static_assert(f == 1);
}