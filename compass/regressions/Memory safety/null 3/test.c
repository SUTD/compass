//Memory safety/null 3

void specify_checks()
{
	check_null();
}

#include <stdlib.h>
int** foo(int size)
{
	int** r = malloc(size*sizeof(int*));
	if(r == NULL) return NULL;
	int i;
	for( i=0; i<size; i++)
	{
		int* p =bar();
		if(p == NULL) continue;
		// ok
		*p = 0;
		r[i] = p;
	}
	
	return r;

		
}