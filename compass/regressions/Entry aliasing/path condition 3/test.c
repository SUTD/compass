

void fff(int* x, int *y, int flag)
{
  if(flag)
  {
    *x = 2;
    *y = 3;
  }
  else
  {
    *y = 3;
    *x = 2;
  }

  static_assert(*y == 2); //ok
}

void call_fff()
{
  int x;
  int y;
 fff(&x, &x, 0);
}