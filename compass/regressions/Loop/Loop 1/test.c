// Loop/Loop 1

#include <stdlib.h>

void foo(int size, int* a)
{
	int i;
	for(i=0; i<size; i++)
	{

	}
	static_assert(i>=size);
	if(size >=0) static_assert(i==size);

}
