void specify_checks()
{
  check_memory_leak();
}

void foo()
{
  int* x = new int;
  delete x;
  
}