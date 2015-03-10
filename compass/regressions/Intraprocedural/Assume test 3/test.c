// Intraprocedural/Assume test 3

#include <stdlib.h>

void foo(int* x, int flag)
{
	assume(!flag  || x!=NULL );
	if(flag) static_assert(x!=NULL);
}