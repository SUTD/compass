// Loop/Termination Cond 3

#include <stdlib.h>

void foo(int size, int* a, int* b)
{
	int i;
	for(i=0; i < size; i++) 
	{
		if(a[i] != NULL) break;
	}
	if(i<size && size>=1) static_assert(a[i] != NULL);
}
