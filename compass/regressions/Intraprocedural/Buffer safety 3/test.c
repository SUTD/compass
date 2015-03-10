// Intraprocedural/Buffer safety 3

#include <stdlib.h>

void foo()
{
	int b[10];
	int* x = &b[-1];
	buffer_safe(x,10);
	int t=1;
	
}
