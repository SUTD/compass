// Interprocedural/Very basic 2

#include <stdlib.h>

void bar(int** a)
{
	**a = 2;
}

void foo(int** b)
{
	bar(b);
	static_assert(**b == 2);
}
