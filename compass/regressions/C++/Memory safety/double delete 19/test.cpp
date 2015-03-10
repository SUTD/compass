void specify_checks()
{
  check_double_delete();
}



void foo(int* x, int* y)
{
  delete x;
  delete y;
}

void bar()
{
  int* a = new int[5];
  foo(a, a);
}