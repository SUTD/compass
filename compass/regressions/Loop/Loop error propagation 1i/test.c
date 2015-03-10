// Loop/Loop error propagation 1i
#include <stdlib.h>

void foo(int size)
{
	int i, j;
	for(i=0, j=0; i<size; i++, j+=2)
	{
		static_assert(j-2*i==0); 
	}
}
