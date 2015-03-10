
int x = 99;




int test()
{
  
  static_assert(x == 0); //should fail
  
}