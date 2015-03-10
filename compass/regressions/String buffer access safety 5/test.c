void specify_checks()
{
  check_buffer();
}

void foo()
{
  char* a = "abcd";
  char b[3];
  
  int i=0;
  for(; a[i] != '\0'; i++)
  {
    b[i] = a[i]; 
  }



}