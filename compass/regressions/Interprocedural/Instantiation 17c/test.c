// Interprocedural/Instantiation 17c
#include <stdlib.h>

void assign(int* a, int *b)
{
	*a = *b;
}

void foo()
{	
	int x[2];
	int b[2];
	x[1] = 3;
	b[1] = 54;
	assign(&x[1], &b[1]);
	static_assert(x[1]== 54);
}