// Intraprocedural/Buffer safety 12

#include <stdlib.h>

void foo()
{
	int b[10];
	int* x = &b[15];
	// should fail
	buffer_safe(x,-5);
	int t=1;
	
}

