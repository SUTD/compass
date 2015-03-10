// Case Study/Array append 3

#include <stdlib.h>

void array_append(int* a,  int size_a, int* b, int size_b, int k)
{	
	
	int i = 0;
	
	int* a_copy = malloc(sizeof(int)*size_a);
	for(i=0; i< size_a; i++) {
		a_copy[i] = a[i];
	}
	
	
	for(i=size_a; i < size_a+size_b; i++)
	{
		a[i] = b[i-size_a];
	}
	
	for(i=0; i<size_a; i++)
	{
		static_assert(a[i] == a_copy[i]);
	}
	
	for(i=size_a; i<size_a+size_b; i++)
	{
		static_assert(a[i] == b[i-size_a]);
	}

}
