
int* bar();

void foo(int** x, int** y)
{
  int* t = bar();
  if(t == 0) exit(1);
  *y = t;

 
}


void foo2(int** a, int** b)
{
  foo(a, b);
  static_assert(*b != 0);

}