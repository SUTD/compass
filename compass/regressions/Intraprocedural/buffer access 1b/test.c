// Intraprocedural/buffer access 1b

#include <stdlib.h>

void foo()
{
	int x[3];
	// should fail
	buffer_safe(x, 3);
	
}
