#include <stdlib.h>

int y;

int* bar();

void foo(int** x)
{
  if( (*x = bar()) == NULL) {
    *x = &y;
  }

  static_assert(*x != NULL);
}

void baz()
{
  int** x = malloc(sizeof(int));
  if(!x) exit(1);
  foo(x);
  static_assert(*x != NULL);
}