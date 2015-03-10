// Loop/Loop 13k

#include <stdlib.h>


int bar(int* a)
{
	return *a==1;
}

void foo(int* a)
{
	int i=1;
	while(bar(a) != 0)
	{
		i++;
		a++;
	}

	static_assert(i>=1);
}