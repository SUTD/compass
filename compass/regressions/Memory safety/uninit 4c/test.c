//Memory safety/uninit 4c
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
	}
	
	// ok
	int* t = a[0];
}
