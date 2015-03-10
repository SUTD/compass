//Loops/Last iteration info 3 buggy 2

#include <stdlib.h>


void foo(int size, int* a)
{
	int i=0;
	for(i=0; i<size; i++)
	{
		if(a[i]== 2) {
			break;
		}
	}
	//should fail
	if(size>0 && a[i] == 2)
		static_assert(i<size);		 
}