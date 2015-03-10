// Interprocedural/Very basic 6

#include <stdlib.h>

void bar(int** a, int flag)
{
	if(flag)
		**a = 2;
}

void foo(int** b, int *c)
{
	assume(*b != c);
	*c = 44;
	**b = 77;
	*b = c;
	bar(b, 0);
	static_assert(**b == 44);
}