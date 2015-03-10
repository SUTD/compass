void specify_checks()
{
  check_uninit();
}

int foo()
{
  if(rand()) return 0;
}

void test()
{
  
  int x= foo();
}