void specify_checks()
{
  check_memory_leak();
}



int* foo()
{
  int* x = new int; //leaked!
  x = new int;
  delete x;
  
}