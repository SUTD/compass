//Basic/Error propagation
#include <stdlib.h>


void foo(int* x)
{

	//should fail
	static_assert(x!=NULL);
	if(x==NULL) exit(1);
	static_assert(x!=NULL);
}

void bar(int* y)
{	

	foo(y);
}
