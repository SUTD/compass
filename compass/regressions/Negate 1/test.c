void foo()
{
  int j=1;
  int i = -j;
  static_assert(i==-1);
}