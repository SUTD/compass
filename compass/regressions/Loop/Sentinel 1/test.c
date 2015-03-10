// Loop/Sentinel 1
#include <stdlib.h>


char* x[] ={"duck", "bunny", "turtle", NULL};

void strcpy(int* a)
{

	int size = 0;
	for(; x[size]!=NULL; size++);
	static_assert(buffer_size(x)>size*8);
	

}
