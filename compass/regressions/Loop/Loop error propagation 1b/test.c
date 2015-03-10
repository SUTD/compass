// Loop/Loop error propagation 1b
#include <stdlib.h>

void foo(int** a, int size)
{
	int i;
	for(i=0; a[i]!=NULL; i++)
	{
		// should fail
		static_assert(a[i] == NULL);
	}
}
