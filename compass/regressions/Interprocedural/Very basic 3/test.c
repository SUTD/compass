// Interprocedural/Very basic 3

#include <stdlib.h>

void bar(int** a)
{
	**a = 2;
}

void foo(int** b)
{
	**b = 77;
	bar(b);
	static_assert(**b == 2);
}
