void specify_checks()
{
  check_memory_leak();
}



int* foo(int flag)
{
  int* x = new int;
  if(flag) return x;
  return 0;
  
}

void bar(int flag)
{
    int* p = foo(flag);
    delete p;

  
}