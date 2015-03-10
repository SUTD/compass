// Interprocedural/Array upgrade 2b

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
	if(t !=0) static_assert(a[0] == 2);

}
