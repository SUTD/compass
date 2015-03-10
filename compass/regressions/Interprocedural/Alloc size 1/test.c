// Interprocedural/Alloc size 1

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
	static_assert(buffer_size(t) == sizeof(int)*10);
	int* b = &t[3];
	static_assert(buffer_size(b) == sizeof(int)*7);
}
