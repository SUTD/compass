// Loop/Sentinel 1 buggy
#include <stdlib.h>


char* x[] ={"duck", "bunny", "turtle", NULL};

void strcpy(int* a)
{

	int size = 0;
	for(; x[size]!=NULL; size++);
	//should fail
	static_assert(buffer_size(x)>((size+1)*8));
	

}
