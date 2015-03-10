
void test(int* a, int size)
{
  
  
  int i;
  int *b = a;
  for(i=0; i<size; i++)
  {
   *a = 2;
    *b = 4;
    static_assert(*a == 2); //should fail
  } 
  
}
