// Interprocedural/Swap 7
#include <stdlib.h>

/*
a -> b
b->a

*a->*a
*b->*b

*/
void swap(int** a, int **b)
{
	int* t = *a;
	*a = *b;
	*b = t;

	int t2 = **a;
	**a = **b;
	**b = t2;
}

void foo(int* a , int* b)
{
	int* t1 = a;
	int* t2 = b;
	*a = 1;
	*b = 2;
	swap(&a, &b);
}

void main()
{
  int a = 5;
  int b = 7;
  foo(&a, &b);

  printf("a: %d  b: %d\n", a, b);
  static_assert(a==2);
  static_assert(b == 1);
}