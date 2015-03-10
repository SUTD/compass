void specify_checks()
{
  check_memory_leak();
}

void foo(int flag)
{
  int* x = new int;
  if(flag) delete x;
  //error
  
}