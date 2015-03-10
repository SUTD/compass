


void foo(int** x)
{
  *x[1] = 2;
  *x[2] = 3;
  int t1 = *x[1];
  int t2 = *x[2];
  static_assert(t1 != t2); //should fail
}

void bar()
{
  int* x[3];
  int a;
  x[0] = x[1] = x[2] = &a;
  foo(x);
  
}