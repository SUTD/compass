// Intraprocedural/buffer_size test 3

#include <stdlib.h>

void foo( int flag, int a, int b)
{
	int size;
	if(flag) size =a;
	else size = b;
	int* x = malloc(sizeof(int)*size);
	int y= buffer_size(x);
	static_assert(y == size*sizeof(*x));
	if(flag) static_assert(y==a*sizeof(int));
	else static_assert(y==b*sizeof(int));
}