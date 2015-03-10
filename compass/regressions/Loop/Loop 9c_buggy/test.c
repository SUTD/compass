// Loop/Loop 9c_buggy

#include<stdlib.h>


void foo(int *a, int**b, int size)
{
	int i;
	a[0] = 5;
	for(i=1; i <size; i++)
	{
		a[i] =0;
	}
	// should fail
	static_assert(a[size] == 0);
	
}
