
int rand ();

void foo()
{
  int a[10];
  for(int i=0; i<10; i++)
  {
    if(rand()) continue;
    a[i] = 0;
  }

  for(int i=0; i<10; i++)
  {
    static_assert(a[i] == 0); // should fail!
  }
}