// Loop/Shift buggy 1 

#include <stdlib.h>

void foo(int size, int* a, int k)
{
	int i = 0;
	assume(k>=1 && k < size-1);
	int old = a[k];
	for(i=0; i < size-1; i++)
	{
		a[i+1] = a[i];
	}
	//should fail
	static_assert(old == a[k-1]);

}
