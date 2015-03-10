// Loop/Loop error propagation 1h
#include <stdlib.h>

void foo(int size)
{
	int i, j;
	size = 1;
	for(i=0, j=0; i<size; i++, j+=2)
	{
		static_assert(i==j); 
	}
}
