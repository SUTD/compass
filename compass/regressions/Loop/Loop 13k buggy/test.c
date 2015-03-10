// Loop/Loop 13k buggy

#include <stdlib.h>



void foo(int* a)
{
	int i=1;
	while(*a==0)
	{
		i++;
		a++;
	}

	static_assert(i==1);
}
