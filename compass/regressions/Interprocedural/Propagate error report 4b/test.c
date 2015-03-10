// Interprocedural/Propagate error report 4b
#include <stdlib.h>

void foo(int size, int* a)
{
	int i;
	if(size >= 0) {
		static_assert(a!=NULL);
	}
}
// should fail
void bar(int x)
{
	foo(x, NULL);	
}