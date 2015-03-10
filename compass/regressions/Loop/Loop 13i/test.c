// Loop/Loop 13i

#include <stdlib.h>


int bar(int a)
{
	return a==1;
}

void foo(int* a)
{
	a[1] = 1;
	int i=1;
	while(bar(a[i]) != 0)
	{
		i++;
	}
	static_assert(i>1);
}
