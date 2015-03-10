// Interprocedural/buffer_size test 4e

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
	foo(q);
}

void baz()
{
	int z[5];
	z[1] = 20;
	bar(z);
}