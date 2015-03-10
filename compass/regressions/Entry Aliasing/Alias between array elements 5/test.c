


int foo(int** x)
{
  *x[1] = 2;
  *x[2] = 3;
  int t1 = *x[1];
  return t1;
}

void bar()
{
  int* x[3];
  int a, b, c;
  x[0] = &a;
  x[1] = &b;
  x[2] = &c;
  int r = foo(x);
  static_assert(r == 2); //ok
  
}