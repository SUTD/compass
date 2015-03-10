

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

  static_assert(*x == 2); //ok
}

void call_fff()
{
  int x;
 fff(&x, &x, 0);
}