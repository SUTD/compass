


void foo()
{
   int a[10];
   int i=0;
   do {
      a[i] =0;
      i++;
    }
   while(i < 10);

   for(int i=0; i<10; i++) {
    static_assert(a[i] == 0);
  }

}