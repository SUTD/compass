// Loop/Loop 12

#include <stdlib.h>


void foo(int *a, int size)
{
	int i=0; 
	while(i<size && a[i]!=0)
	{
		i++;
	}
	if(i==0) static_assert(size <=0 || a[0] ==0);

}
