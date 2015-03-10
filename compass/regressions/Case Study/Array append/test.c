// Case Study/Array append

#include <stdlib.h>

void array_append(int* a,  int size_a, int* b, int size_b, int k)
{
	int i = size_a;
	for(; i < size_a+size_b; i++)
	{
		a[i] = b[i-size_a];
	}
	if(k >=size_a && k < size_a+size_b)
		static_assert(a[k] == b[k-size_a]);
}
