
int rand ();

void foo()
{
  int a[10];
  for(int i=0; i<10; i++)
  {
    a[i] = 0;
    if(rand()) continue;
   

  }

  for(int i=0; i<10; i++)
  {
    static_assert(a[i] == 0); // correct
  }
}