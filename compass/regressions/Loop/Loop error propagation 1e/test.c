// Loop/Loop error propagation 1e
#include <stdlib.h>

void foo(int** a, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		static_assert(i<=size);
	}
}
