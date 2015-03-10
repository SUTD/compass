// Intraprocedural/Buffer safety 9

#include <stdlib.h>

void foo()
{
	int b[10];
	int* x = &b[15];
	buffer_safe(x,-10);
	int t=1;
	
}
