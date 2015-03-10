
int* bar();

int* foo()
{
  int* x = bar();
  
  static_assert( x !=0); //should fail

  if(x == 0) exit(1);
  return x;
}


void foo2()
{
  int* t = foo(); 
}