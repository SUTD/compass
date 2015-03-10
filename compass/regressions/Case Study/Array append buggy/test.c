// Case Study/Array append buggy

#include <stdlib.h>

void array_append(int* a,  int size_a, int* b, int size_b, int k)
{
	int i = size_a;
	for(; i < size_a+size_b; i++)
	{
		a[i] = b[i-size_a];
	}
	
	// should fail
	for(i=0; i<size_a+size_b; i++)
	{
		static_assert(a[i] == b[i-size_a]);
	}

}
