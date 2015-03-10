// Loop/Imprecision test 4

#include <stdlib.h>

void foo(int flag, int* x)
{
	int i = 0;
	while(malloc(4) !=0)
	{
		i++;
	}
	static_assert(i>=0);
	
}
