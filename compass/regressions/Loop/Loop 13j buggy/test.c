// Loop/Loop 13j buggy

#include <stdlib.h>



void foo(int* a)
{
	a[1] = 1;
	int i=1;
	while(a[i]==1)
	{
		i++;
	}
	// should fail
	static_assert(0);
}
