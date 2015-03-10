// Loop/Loop 13g

#include <stdlib.h>


int bar(int a)
{
	return a==1;
}

void foo(int* a)
{
	int i=1;
	while(bar(a[i]) != 0)
	{
		i++;
	}
	
	static_assert(i>=1);
}
