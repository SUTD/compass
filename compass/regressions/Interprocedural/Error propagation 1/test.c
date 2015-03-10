// Interprocedural/Error propagation 1

#include <stdlib.h>

void foo(int* a)
{
	static_assert(a!=NULL);
	
}

void bar(int* a, int flag)
{
	int* p =malloc(4);
	if(p == NULL) return;
	foo(p);
	
}
