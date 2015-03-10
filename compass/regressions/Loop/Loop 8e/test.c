// Loop/Loop 8e

#include<stdlib.h>

void foo(int** a, int size, int elem, int flag)
{
	int i;
	for(; *a !=NULL;)
	{
		a++;
	}
	assume(*a == NULL);
	static_assert(*a ==NULL);

}
