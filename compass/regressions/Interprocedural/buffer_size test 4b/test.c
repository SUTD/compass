// Interprocedural/buffer_size test 4b

#include <stdlib.h>

void foo(int* x)
{
	int size;
	size =buffer_size(x);
	//should fail
	static_assert(size>9*sizeof(int));
}

void bar()
{
	int* q = malloc(sizeof(int)*8);
	foo(q);
}