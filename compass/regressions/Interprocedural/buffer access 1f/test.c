// Interprocedural/buffer access 1f

#include <stdlib.h>

void foo(int* x, int y)
{
	//should fail
	buffer_safe(x, y);
	
}

void bar()
{
	int x[4];
	foo(x, 4);
}
