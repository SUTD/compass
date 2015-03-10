void specify_checks()
{
  check_null();
}


void foo(int* x)
{
  int* y;
  for(y=x; *y; y++) // should report error
  {

  }
}