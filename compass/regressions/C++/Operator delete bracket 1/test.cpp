class Foo
{
   public:
       int x;
       Foo(){x=3;}
       ~Foo(){x=0;}
};


void test()
{
      Foo* f = new Foo[2];
      delete[] f;
      static_assert(f[1].x == 0);
}