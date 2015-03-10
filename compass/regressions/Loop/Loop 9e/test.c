// Loop/Loop 9e

#include<stdlib.h>


void foo(int *a, int**b, int size)
{
	int i;
	a[0] = 5;
	a[1] = 2;
	for(i=1; i <size; i++)
	{
		a[i] =0;
	}
	static_assert(a[0] == 5);
	if(size > 1) static_assert(a[1] == 0);
	
}
