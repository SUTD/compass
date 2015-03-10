// Loop/Termination Cond 2

#include <stdlib.h>

void foo(int size, int* a, int* b)
{
	int i = 0;
	while(1)
	{
		if(a[i] != NULL) break;
		i++;
	}
	static_assert(a[i] != NULL);
}
