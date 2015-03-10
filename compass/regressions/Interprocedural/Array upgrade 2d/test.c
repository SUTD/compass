// Interprocedural/Array upgrade 2d

#include <stdlib.h>

void zoo(int** a)
{
	if(**a == 0) return;
	**a = 5;
	(*a)++;	
	
}

void foo(int *a, int *b, int* c)
{
	int t = *a;
	a[1] = 2;
	zoo(&a);

	// SHOULD FAIL
	if(t !=0) static_assert(a[0] == 5);

}
