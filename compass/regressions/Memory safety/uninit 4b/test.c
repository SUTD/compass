//Memory safety/uninit 4b
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
		*a[i] = 0;
		int t = *a[i];
	}
}