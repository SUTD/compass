void specify_checks()
{
  check_memory_leak();
}



int* foo()
{
  int* x = new int; 
  int* y = x;
  x = new int;
  delete x;
  delete y; // no leaks
  
}

