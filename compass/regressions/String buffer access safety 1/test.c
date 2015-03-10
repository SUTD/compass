void specify_checks()
{
  check_buffer();
}

void foo()
{
  char* a = malloc(5);
  a[0] = a[1] = a[2] = a[3] = 'c';
  a[4] = '\0';
  char b[5];
  
  int i=0;
  for(; a[i] != '\0'; i++)
  {
    b[i] = a[i];
  }
}