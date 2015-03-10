// Intraprocedural/buffer_size test 2

#include <stdlib.h>

void foo(int size)
{
	int* x = malloc(sizeof(int)*size);
	int y= buffer_size(x);
	static_assert(y == size*sizeof(*x));
}