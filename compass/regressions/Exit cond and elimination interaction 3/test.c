
int* bar();

void foo(int** x, int** y)
{
  int* t = bar();
  *x = t;
  if(t != 0) *y = t;

 
}


void foo2(int** a, int** b)
{
  foo(a, b);
  static_assert(*a != 0); //should fail

}