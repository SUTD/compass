// Loop/Imprecision in increment 2

#include <stdlib.h>



void foo(int size)
{
	int i=1;
	int j=1;
	while(i<size)
	{
		 i++;
		 j++;
	}

	if(size >=1) static_assert(i==j);
}
