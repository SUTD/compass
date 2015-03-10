// Case Study/Merge interleave

#include <stdlib.h>

void merge_interleave(int* res, int* a, int*b, int size)
{
	int i;
	for(i=0; i < size; i+=2)
	{
		res[i] = a[i];
	}
	for(i=1; i < size; i+=2)
	{
		res[i] = b[i];
	}
	int k;
	if( k>=0 && k<size) {
		if(k%2 == 0) static_assert(res[k] == a[k]);
		else static_assert(res[k] == b[k]);
	}
}


