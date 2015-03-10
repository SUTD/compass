// Loop/Loop 12b

#include <stdlib.h>


void foo(int *a, int size)
{
	int i=0; 
	while(i<size && a[i]!=0)
	{
		i++;
	}
	//should fail
	if(i==0) static_assert(a[0] ==0);

}
