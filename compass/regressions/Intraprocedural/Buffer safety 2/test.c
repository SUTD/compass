// Intraprocedural/Buffer safety 2

#include <stdlib.h>

void foo()
{
	int b[10];
	int* x = &b[-1];
	
	//should fail
	buffer_safe(x,0);
	int t=1;
	
}

