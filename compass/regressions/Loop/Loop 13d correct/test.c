// Loop/Loop 13d correct

#include <stdlib.h>

void foo()
{
	int i=0;
	while(unknown() != 0)
	{
		i++;
	}
	static_assert(i>=0);
}
