// Loop/Loop 9c

#include<stdlib.h>


void foo(int *a, int**b, int size)
{
	int i;
	a[0] = 5;
	for(i=1; i <size; i++)
	{
		a[i] =0;
	}
	static_assert(a[0] == 5);
	
}
