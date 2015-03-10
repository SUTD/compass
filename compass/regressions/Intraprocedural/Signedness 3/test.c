// Intraprocedural/Signedness 3

#include <stdlib.h>

void foo(int* i)
{
	// should fail
	if(*i < 0) static_assert(0);
	
}
