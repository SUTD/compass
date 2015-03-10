// Intraprocedural/Buffer safety 6

#include <stdlib.h>

void foo()
{
	int b[10];
	int* x = &b[1];
	// should fail
	buffer_safe(x,-2);
	int t=1;
	
}

