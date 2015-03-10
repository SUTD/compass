// Loop/Loop 8_buggy

#include<stdlib.h>

void foo(int** a, int size, int elem, int flag)
{
	int i;
	for(i=0; a[i]!=NULL; i++)
	{
		if(i>size) break;
	}
	// should fail
	static_assert(a[i]==NULL);

}
