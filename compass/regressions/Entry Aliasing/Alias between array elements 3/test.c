


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
  int a;
  x[0] = x[1] = x[2] = &a;
  int r = foo(x);
  static_assert(r == 3); // ok
  
}