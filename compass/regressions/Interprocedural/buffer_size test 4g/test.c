// Interprocedural/buffer_size test 4g

#include <stdlib.h>

void foo(int* x)
{
	int size;
	size =buffer_size(x);
	static_assert(size>9*sizeof(int));
}

void bar(int* z)
{
	assume(z[1]==10);
	int* q = malloc(sizeof(int)*z[1]);
	foo(q);
}