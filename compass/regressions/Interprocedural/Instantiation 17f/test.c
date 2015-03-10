// Interprocedural/Instantiation 17f
#include <stdlib.h>

void swap(int* a, int *b)
{
	int t = a[1];
	a[1] = b[1];
	b[1] = t;
}

void foo()
{	
	int x[2];
	int b[2];
	x[1] = 45;
	b[1] = 54;
	assign(&x[0], &b[0]);
	static_assert(x[1]== 45);
	static_assert(b[1]== 54);
}