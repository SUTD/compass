int foo(int** x, int** y, int* p, int* q)
{
  *q = 3;
  *p= 7;
  *x = p;
  *y = q;
  return **x;
}

void main()
{
  
  int a =4; int b=5;
  int* p = &a;
  int* q = &b;

  int c = 1;
  int d = 1;

  int* cc = &c;
  int* dd = &d;

  int** x = &cc;
  int** y = &dd;

  int r = foo(x, y, p, q);
  printf("R is: %d\n", r);
  static_assert(r == 7); //ok
 

}


