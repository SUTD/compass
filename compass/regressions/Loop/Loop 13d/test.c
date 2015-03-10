// Loop/Loop 13d

#include <stdlib.h>

void foo()
{
	int i=0;
	while(unknown() != 0)
	{
		i++;
	}
	// should fail
	static_assert(i>0);
}
