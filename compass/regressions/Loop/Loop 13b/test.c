// Loop/Loop 13b

#include <stdlib.h>

void foo()
{
	int i=0;
	while(malloc(4) != 0)
	{
		i++;
	}
	//should fail
	static_assert(i>0);
}
