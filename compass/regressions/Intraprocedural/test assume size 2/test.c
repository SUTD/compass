// Intraprocedural/test assume size 2

#include <stdlib.h>

void foo(int flag, int* x)
{
	assume(buffer_size(x) == 4);
	static_assert(buffer_size(x) == 4);
	
}
