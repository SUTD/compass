// Intraprocedural/test assume size 2b

#include <stdlib.h>

void foo(int flag, int* x)
{
	assume(buffer_size(x) == 4);
	// should fail
	static_assert(buffer_size(x) == 3);
	
}
