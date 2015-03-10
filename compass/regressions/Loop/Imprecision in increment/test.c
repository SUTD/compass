// Loop/Imprecision in increment

#include <stdlib.h>



void foo(int size)
{
	int i=1;
	int j=1;
	while(i<size)
	{
		 i++;
		 if(i==3) j++;
	}
	// should fail
	if(size >=1) static_assert(i==j);
}
