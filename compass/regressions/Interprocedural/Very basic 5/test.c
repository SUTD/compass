// Interprocedural/Very basic 5

#include <stdlib.h>

void bar(int** a, int flag)
{
	if(flag)
		**a = 2;
}

void foo(int** b, int *c)
{
	**b = 77;
	*b = c;
	bar(b, 1);
	static_assert(**b == 2);
}
