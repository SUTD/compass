
int* bar();

int foo(int** x, int** y)
{
  int* t = bar();
  if(t == 0) return -1;
  *y = t;
  return 0;

 
}


void foo2(int** a, int** b)
{
  int r = foo(a, b);
  if(r ==0) static_assert(*b != 0); 

}