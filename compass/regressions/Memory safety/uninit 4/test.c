//Memory safety/uninit 4
#include <stdlib.h>

void specify_checks()
{
	check_uninit();
}

void foo()
{
	int i;
	int** a= malloc(10*sizeof(int));
	for(i=0; i<10; i++)
	{
		a[i] = malloc(sizeof(int));
		// uninitialized read
		int t = *a[i];
	}
}
