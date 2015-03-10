// Loop/Loop error propagation 1
#include <stdlib.h>

void foo(int** a, int size)
{
	int i;
	for(i=0; a[i]!=NULL; i++)
	{
		static_assert(a[i] != NULL);
	}
}
