// Interprocedural/Instantiation 17
#include <stdlib.h>

void assign(int* a, int *b)
{
	*a = *b;
}

void foo()
{	
	int x;
	int b[2];
	b[0] = 54;
	assign(&x, &b[0]);
	static_assert(x== 54);	
}
