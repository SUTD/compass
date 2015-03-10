
void foo()
{
  int a[10];
  for(int i=0; i<8; i++) {
    a[i] = 0;
   }
    a[8] = a[9] =1;

   for(int i=0; i<10; i++) {
      static_assert(a[i] == 0); // should fail!
   }
}