// Interprocedural/Swap 1
#include <stdlib.h>

void swap(int* a, int *b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

void foo()
{
	int a = 45;
	int b= 54;
	swap(&a, &b);
	static_assert(a == 54);	
	static_assert(b==45);
}
