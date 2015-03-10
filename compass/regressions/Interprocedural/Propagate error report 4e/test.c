// Interprocedural/Propagate error report 4e
#include <stdlib.h>

void foo(int* a, int x)
{
	int i;
	if(a!=NULL && *a > 0)
		static_assert(x!=-1);
}

void bar(int size)
{
	int* x = malloc(sizeof(int)*size);
	if(x==NULL)
		foo(x, size);
}