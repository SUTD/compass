// Loop/Loop breakpoint test 1

#include <stdlib.h>

void foo(int flag, int x)
{
	while(1)
	{
		//should fail
		static_assert(x<=3);
		if(x>3) break;
	}
	
}