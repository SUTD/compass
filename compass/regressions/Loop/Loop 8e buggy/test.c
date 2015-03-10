// Loop/Loop 8e buggy

#include<stdlib.h>

void foo(int** a, int size, int elem, int flag)
{
	int i;
	for(; *a !=NULL;)
	{
		a++;
	}
	// should fail
	static_assert(*a !=NULL);

}
