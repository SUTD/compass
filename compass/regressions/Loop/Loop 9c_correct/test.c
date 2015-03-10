// Loop/Loop 9c_correct

#include<stdlib.h>


void foo(int *a, int**b, int size)
{
	int i;
	a[0] = 5;
	for(i=1; i <size; i++)
	{
		a[i] =0;
	}
	if(size>=2) static_assert(a[size-1] == 0);
	
}
