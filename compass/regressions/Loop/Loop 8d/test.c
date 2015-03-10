// Loop/Loop 8d

#include<stdlib.h>

void foo(int** a, int** b, int size, int elem, int flag)
{
	int i;
	for(i=0; a[i]!=NULL; i++)
	{
		b[i] = a[i];
	}
	
	// SHOULD FAIL (a[0] = NULL, but b[0] is not NULL.) 
	for(i=0; b[i] != NULL; i++)
	{
		static_assert(b[i] == a[i]);
	}



}
