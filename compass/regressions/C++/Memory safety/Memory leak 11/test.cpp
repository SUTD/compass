void specify_checks()
{
  check_memory_leak();
}



void foo(int** a, int size)
{
  for(int i=0; i<size; i++)
  {
    a[i] = new int;
  }

  for(int i=0; i<size; i++)
   {
    delete a[i];
  } 
  // no leaks
  
}