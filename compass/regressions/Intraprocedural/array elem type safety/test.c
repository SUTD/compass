// Intraprocedural/array elem type safety

#include <stdlib.h>

// should report type safety error
void foo(int* x)
{
	x[3] = 7;
	char* y = (char*)x;
	y[3] = 'a';
	int b = 2;
	
}
