// Interprocedural/buffer access 1e

#include <stdlib.h>

void foo(int* x, int y)
{
	buffer_safe(x, y);
	
}

void bar()
{
	int x[4];
	foo(x, 3);
}