// Interprocedural/buffer access 1d

#include <stdlib.h>

void foo(int y)
{
	int x[3];
	// should fail
	buffer_safe(x, y);
	
}

void bar()
{
	foo(3);
}
