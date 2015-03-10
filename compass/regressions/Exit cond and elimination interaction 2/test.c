
int* bar();

int* foo()
{
  int* x = bar();
  if(x == 0) exit(1);
  
  static_assert( x !=0); //should not fail


  return x;
}


void foo2()
{
  int* t = foo(); 
}