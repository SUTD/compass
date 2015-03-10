// Interprocedural/Error propagation 2

#include <stdlib.h>

void foo(int* a)
{
	static_assert(a!=NULL);
	
}

void bar(int* a, int flag)
{
	// should fail
	foo(0);
	
}
