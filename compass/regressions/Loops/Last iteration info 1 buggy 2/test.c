//Loops/Last iteration info 1 buggy 2

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
	if(a==2) {
		static_assert(i<size);
	}
		 
}