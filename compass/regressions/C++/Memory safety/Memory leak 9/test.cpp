void specify_checks()
{
  check_memory_leak();
}



void foo()
{
  int* a[10];
  for(int i=0; i<10; i++)
  {
    a[i] = new int;
  }

  for(int i=0; i<10; i++)
   {
    delete a[i];
  } 

  // no leaks
  
}