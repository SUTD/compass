// Intraprocedural/Buffer safety 1

#include <stdlib.h>

void foo()
{
	int b[10];
	int* x = &b[-1];
	buffer_safe(x,1);
	int t=1;
	
}
