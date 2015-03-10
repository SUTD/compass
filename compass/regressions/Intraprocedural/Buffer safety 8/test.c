// Intraprocedural/Buffer safety 8

#include <stdlib.h>

void foo()
{
	int b[10];
	int* x = &b[1];
	buffer_safe(x,8);
	int t=1;
	
}
