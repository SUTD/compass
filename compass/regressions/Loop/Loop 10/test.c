// Loop/Loop 10

#include<stdlib.h>

void foo(int* a, int size, int* elem, int j)
{
	int i;
	int x = -1;
	for(i=0; i<size; i++)
	{
		if(i==1) a[i]=3;
		else a[i] =2;
	}
	if(j>=0 && j !=  1 && j<size) static_assert(a[j] ==2);
	else if( j==1 && size >=2) static_assert(a[j]==3);

	// This should fail:
	if( j==1 && size >=1) static_assert(a[j]==3);


}
