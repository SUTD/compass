// Interprocedural/buffer_size test 4

#include <stdlib.h>

void foo(int* x)
{
	int size;
	size =buffer_size(x);
	static_assert(size>2);
}

void bar()
{
	int* q = malloc(sizeof(int)*8);
	foo(q);
}