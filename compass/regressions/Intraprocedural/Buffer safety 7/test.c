// Intraprocedural/Buffer safety 7

#include <stdlib.h>

void foo()
{
	int b[10];
	int* x = &b[1];
	// should fail
	buffer_safe(x,9);
	int t=1;
	
}

