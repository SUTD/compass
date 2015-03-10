// Intraprocedural/buffer access 1

#include <stdlib.h>

void foo()
{
	int x[3];
	buffer_safe(x, 2);
	
}