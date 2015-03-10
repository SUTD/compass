// Interprocedural/Propagate error report 4d
#include <stdlib.h>

void foo(int* a, int x)
{
	int i;
	if(a!=NULL && *a > 0)
		static_assert(x!=-1);
}
// should fail
void bar(int size)
{
	int* x = malloc(sizeof(int)*size);
	foo(x, size);
}