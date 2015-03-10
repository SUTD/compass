// Loop/Loop error propagation 1j
#include <stdlib.h>

void foo(int size)
{
	int i, j;
	for(i=1, j=0; i<size; i++, j+=2)
	{
		//SHOULD FAIL
		static_assert(j-2*i==0); 
	}
}