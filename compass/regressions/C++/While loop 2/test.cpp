
void foo()
{
  
  int a[10];
 int i=0;
 while(i<10) {
   a[i] = 0;
   i++;
 } 
  
  for(int i=0; i<11; i++)
  {
    static_assert(a[i] == 0); //should fail
  }

}