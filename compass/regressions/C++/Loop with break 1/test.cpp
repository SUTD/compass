
void foo()
{
  
 int i;
for(i=0; ; i++)
 {
   if(i==10) break;
 } 

  static_assert(i==10);
}