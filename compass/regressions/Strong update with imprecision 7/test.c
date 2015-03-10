#include <stdlib.h>

int y;

int* bar();

void foo(int** x)
{
  if( (*x = bar()) == NULL && rand()) {
    *x = &y;
  }
  //should fail
  static_assert(*x != NULL);
}

void baz()
{
  int** x = malloc(sizeof(int));
  if(!x) exit(1);
  foo(x);
  //should fail
  static_assert(*x != NULL);
}