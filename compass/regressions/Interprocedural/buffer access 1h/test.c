// Interprocedural/buffer access 1h

#include <stdlib.h>

void foo(int* x, int* y)
{
	// should fail
	buffer_safe(x, y[1]);
	
}

void bar()
{
	int x[4];
	int y[4];
	y[1] = 4;
	foo(x, y);
}
