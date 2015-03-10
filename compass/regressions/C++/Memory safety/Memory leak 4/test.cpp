void specify_checks()
{
  check_memory_leak();
}

int* foo()
{
  int* x = new int;
  return x;
  
}

void bar()
{
  int* p = foo();
  delete p;
}