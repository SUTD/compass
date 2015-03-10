void specify_checks()
{
  check_buffer();
}

void foo()
{
  char* a = "abcd";
  char x = a[1];
  char b[4];
  
  int i=0;
  for(; a[i] != '\0'; i++)
  {
    b[i] = a[i]; 
  }

  b[i] = '\0'; // should fail

}