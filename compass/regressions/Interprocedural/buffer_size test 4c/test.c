// Interprocedural/buffer_size test 4c

#include <stdlib.h>

void foo(int* x)
{
	int size;
	size =buffer_size(x);
	static_assert(size>9*sizeof(int));
}

void bar(int* z)
{
	int* q = malloc(sizeof(int)*z[1]);
	if(z[1]>10)
		foo(q);
}