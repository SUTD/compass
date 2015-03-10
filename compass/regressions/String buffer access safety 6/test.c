void specify_checks()
{
  check_buffer();
}

void foo(char* a, char* b)
{
  
  int i=0;
  for(; a[i] != '\0'; i++)
  {
    b[i] = a[i]; 
  }
  b[i] = '\0';

}

void good()
{
  char* a = "abcd";
  char x = a[2];
  char* b = malloc(5);
  foo(a, b);
}
