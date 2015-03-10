// Loop/Loop 9b

#include<stdlib.h>


void foo(int **a, int size)
{
	int i;
	for(i=0; i <size; i++)
	{
		a[i] = malloc(4);
		*a[i] = 0;
	}

	for(i=0; i<size; i++)
	{
		static_assert(*a[i] == 0);
	}

}
