// Interprocedural/Instantiation 17d
#include <stdlib.h>

void assign(int* a, int *b)
{
	a[1] = *b;
}

void foo()
{	
	int x[3];
	int b[2];
	b[1] = 54;
	assign(&x[1], &b[1]);
	static_assert(x[2]== 54);
}