void specify_checks()
{
  check_memory_leak();
}



int* foo(int flag, int* y)
{
  *y = 0;
  int* x = new int;
  if(flag) return x;
  else delete x;
  return 0;
  
}

void bar(int flag)
{
  int c;
  int* p = foo(flag, &c);
  delete p; //no leak

  
}