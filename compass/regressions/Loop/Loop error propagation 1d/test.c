// Loop/Loop error propagation 1d
#include <stdlib.h>

void foo(int** a, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		// should fail
		static_assert(i==size);
	}
}
