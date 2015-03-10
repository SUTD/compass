// Loop/Array assigned to imprecise value 2

#include <stdlib.h>

void foo(int size, int flag, int* a, int* b)
{
	int i=0;
	while(i < size) 
	{
		i=i+flag;
		a[i] = 4;
	}
	//should fail
	if(size>=2)
		static_assert(a[0] == a[1]);

}
