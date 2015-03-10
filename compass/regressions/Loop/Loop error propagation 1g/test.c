// Loop/Loop error propagation 1g
#include <stdlib.h>

void foo(int size)
{
	int i, j;
	for(i=0, j=0; i<size; i++, j+=2)
	{
		// SHOULD FAIL
		static_assert(i==j); 
	}
}
