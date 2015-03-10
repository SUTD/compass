// Loop/Loop error propagation 1f
#include <stdlib.h>

void foo(int size)
{
	int i, j;
	for(i=0, j=0; i<size; i++, j++)
	{
		static_assert(i==j);
	}
}
