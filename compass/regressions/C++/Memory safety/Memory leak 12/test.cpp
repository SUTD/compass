void specify_checks()
{
  check_memory_leak();
}



void foo(int** a, int size)
{
  for(int i=0; i<size; i++)
  {
    a[i] = new int; // last element is leaked
  }

  for(int i=0; i<size-1; i++)
   {
    delete a[i];
  } 

  
}