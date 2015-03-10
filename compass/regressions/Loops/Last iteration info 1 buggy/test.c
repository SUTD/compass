//Loops/Last iteration info 1 buggy

#include <stdlib.h>


void foo(int size, int a)
{
	int i=0;
	for(i=0; i<size; i++)
	{
		if(a== 2) {
			break;
		}
	}
	if(size>=1 &&a==2) {
		// should fail
		static_assert(i<size-1);
	}
		 
}