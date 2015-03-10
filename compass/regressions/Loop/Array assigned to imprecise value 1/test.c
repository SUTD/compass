// Loop/Array assigned to imprecise value 1

#include <stdlib.h>

void foo(int size, int flag, int* a, int* b)
{
	int i=0;
	while(i < size) 
	{
		i=i+flag;
		a[i] = i;
	}
	//should fail
	if(size>=2)
		static_assert(a[0] == a[1]);

}
