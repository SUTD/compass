// Loop/Basic Pointer arithmetic 1 buggy 2

#include <stdlib.h>

void foo(int size, int* a)
{
	int i;
	int* a_copy = a;
	for(i=0; i < size; i++)
	{
		*a = 0;
		a++;
	}
	for(i=0; i < size; i++)
	{
		//should fail
		static_assert(a[i] == 0);
	}
}
