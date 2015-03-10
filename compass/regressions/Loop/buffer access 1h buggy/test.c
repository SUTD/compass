// Loop/buffer access 1h buggy

#include <stdlib.h>

void foo(int* x, int* y)
{
	buffer_safe(x, y[1]);
	
}

void bar()
{
	int x[4];
	int y[4];
	y[1] = 4;
	// should fail
	foo(x, y);
}
