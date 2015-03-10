// Interprocedural/Very basic 4

#include <stdlib.h>

void bar(int** a)
{
	**a = 2;
}

void foo(int** b, int *c)
{
	**b = 77;
	*b = c;
	bar(b);
	static_assert(**b == 2);
}
