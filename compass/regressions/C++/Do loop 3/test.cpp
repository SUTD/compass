



void specify_checks()
{
  check_buffer();
}

void foo()
{
   int a[10];
   int i=0;
   do {
      i++;
      a[i] =0; // buffer overrun!
    }
   while(i < 10);



}