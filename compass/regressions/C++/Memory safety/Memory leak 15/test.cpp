void specify_checks()
{
  check_memory_leak();
}


void bar(int** a, int size)
{
  for(int i=0; i<size; i++)
   {
    delete a[i];
  } 
}

void foo(int** a, int size)
{
  for(int i=0; i<size; i++)
  {
    a[i] = new int;
  }
  
}

void baz()
{
  int** a = new int*[10];
  foo(a, 10);
  bar(a, 10);
  delete[] a; // no leaks
}