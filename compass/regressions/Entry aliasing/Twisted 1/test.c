int foo(int** x, int** y, int* p, int* q)
{
  *p= 7;
  *q = 3;
  *x = p;
  *y = q;
  return **x;
}

void main()
{
  
  int a =4; int b=5;
  int* p = &a;
  int* q = &b;
  int** x = &p;
  int** y = &p;

  int r = foo(x, y, p, q);
  printf("R is: %d\n", r);
  static_assert(r == 3);
 

}


