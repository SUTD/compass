// Loop/Termination Cond 3a

#include <stdlib.h>

void foo(int size, int flag, int* a, int* b)
{
	int i=0;
	while(i < size) 
	{
		i=i+flag;
	}
	//should fail
	static_assert(0);

}
