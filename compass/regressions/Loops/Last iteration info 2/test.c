//Loops/Last iteration info 2

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
	if(size>0 && i<size) static_assert(a == 2); 
		 
}