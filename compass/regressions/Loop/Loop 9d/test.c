// Loop/Loop 9d

#include<stdlib.h>


void foo(int *a, int**b, int size)
{
	int i;
	a[0] = 5;
	a[1] = 2;
	for(i=2; i <size; i++)
	{
		a[i] =0;
	}
	static_assert(a[0] == 5);
	static_assert(a[1] == 2);
	
}
