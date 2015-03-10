



void specify_checks()
{
  check_buffer();
}

void foo()
{
   int a[10];
   int i=0;
   do {

      a[i] =0; // ok
      i++;
    }
   while(i < 10);



}