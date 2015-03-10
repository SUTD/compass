// Loop/Loop 9e_buggy

#include<stdlib.h>


void foo(int *a, int**b, int size)
{
	int i;
	a[0] = 5;
	a[1] = 2;
	for(i=0; i <size; i++)
	{
		a[i] =0;
	}
	// should fail
	static_assert(a[0] == 5);
	
}
