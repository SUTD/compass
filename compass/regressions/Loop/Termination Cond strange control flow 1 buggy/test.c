// Loop/Termination Cond strange control flow 1 buggy

#include <stdlib.h>

void foo(int size, int flag, int* a, int* b)
{
	int i=0;

	
	i = 0;
	while(1)
	{
		i++;
		if(i>= size-1) break;
	}
	//should fail
	static_assert(i==size-1);
	
	
	

}

