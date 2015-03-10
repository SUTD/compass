// Loop/Loop breakpoint test 2

#include <stdlib.h>

void foo(int size, int x)
{
	int i =0;
	for(i=0; i<size; i++)
	{
		if(x>3) break;
	}
	//should fail
	static_assert(x<=3);
	
}