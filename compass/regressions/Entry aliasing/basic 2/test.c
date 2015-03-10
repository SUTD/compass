





void foo(int** x, int** y)
{
  *x = malloc(sizeof(int));
  *y = malloc(sizeof(int));
  **y = 3;
  **x = 2;
}

void bar(int** x)
{
  int** y =x;
  foo(x, y);
  static_assert(**y == 3); //should fail
}

