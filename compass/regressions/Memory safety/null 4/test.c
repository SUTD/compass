void specify_checks()
{
  check_null();
}


void foo(int* x)
{
  assume(x != 0);
  int* y;
  for(y=x; *y; y++)
  {

  }
}