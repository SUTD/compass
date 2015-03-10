// Loop/Imprecision test 4b

#include <stdlib.h>

void foo(int flag, int* x)
{
	int i = 0;
	while(malloc(4) !=0)
	{
		i++;
	}
	// should fail
	static_assert(i>=1);
	
}