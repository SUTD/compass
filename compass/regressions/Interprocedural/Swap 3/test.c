// Interprocedural/Swap 3
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
	swap(&a, &a);
	swap(&b, & b);
	static_assert(a == 45);	
	static_assert(b==54);
}