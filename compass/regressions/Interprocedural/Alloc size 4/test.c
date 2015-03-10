// Interprocedural/Alloc size 4

#include <stdlib.h>

int *alloc()
{
	int * res = malloc(10*sizeof(int));
	res[3] = 4;
	return res;
}


void foo()
{
	int* t = alloc();
	int* b = &t[3];
	buffer_safe(b,6);
}
