void specify_checks()
{
  check_null();
}

void foo(int** a)
{
  for(; ;)
  {
    if(*a) exit(1); //should fail!
  }
}