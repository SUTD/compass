void foo(int x)
{
  int i;
  int a[10];
  for(i=0; i<10;i++)
  {
    a[i] = i;
  }
 
  static_assert(a[0] == a[1]);


}