//Memory safety/uninit 4j
#include <stdlib.h>

void specify_checks()
{
	check_uninit();
}

void foo()
{
	int i;
	int** a= malloc(10*sizeof(int));
	int b[10];
	for(i=0; i<10; i++)
	{
		a[i] = malloc(sizeof(int));
	}
	
	for(i=0; i<10; i++)
	{
		*a[i] = 0;
	}
	
	for(i=0; i<5; i++)
	{
		b[i] = *a[i];
	}
	
	// not ok
	int t = b[5];
	
	
}
