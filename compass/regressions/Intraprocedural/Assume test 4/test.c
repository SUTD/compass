// Intraprocedural/Assume test 4

#include <stdlib.h>

void foo(int* x, int flag)
{
	assume(!flag  || x!=NULL );
	// should fail
	static_assert(x!=NULL);
}