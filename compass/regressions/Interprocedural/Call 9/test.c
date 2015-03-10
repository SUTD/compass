// Interprocedural/Call 9
#include <stdlib.h>



int* bar(int* a)
{
	int* c = malloc(sizeof(int));
	*c = *a;
	*a = 22;
	return c;
}


void foo()
{
	int * b = malloc(sizeof(int));
	*b = 55;
	 int* ret = bar(b);
	static_assert(*ret == 55);
	static_assert(*b == 22);
}
