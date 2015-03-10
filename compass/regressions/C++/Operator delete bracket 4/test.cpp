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
      for(int i=0; i<=2; i++) {
	  static_assert(f[i].x == 0); //should fail
      }
}