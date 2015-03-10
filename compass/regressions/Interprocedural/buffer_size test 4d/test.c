// Interprocedural/buffer_size test 4d

#include <stdlib.h>

void foo(int* x)
{
	int size;
	size =buffer_size(x);
	
	// should fail
	static_assert(size>9*sizeof(int));
}

void bar(int* z)
{
	int* q = malloc(sizeof(int)*z[1]);
	foo(q);
}