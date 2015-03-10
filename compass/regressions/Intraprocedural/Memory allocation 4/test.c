// Intraprocedural/Memory allocation 4

#include <stdlib.h>

void foo(int* x, int i)
{

	x = malloc(sizeof(int)*i);
	x[i-1] = 77;
	int last_elem = i-1;
	static_assert(x[last_elem] == 77);
	
	

}