// Intraprocedural/Buffer safety 11

#include <stdlib.h>

void foo()
{
	int b[10];
	int* x = &b[15];
	buffer_safe(x,-6);
	int t=1;
	
}
