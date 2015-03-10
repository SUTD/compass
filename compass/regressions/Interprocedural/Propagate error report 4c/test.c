// Interprocedural/Propagate error report 4c
#include <stdlib.h>

void foo(int size, int* a)
{
	int i;
	if(size >0) {
		static_assert(a!=NULL);
	}
}

// should fail
void bar(int size)
{
	int* x = malloc(sizeof(int)*size);
	foo(size, x);	
}