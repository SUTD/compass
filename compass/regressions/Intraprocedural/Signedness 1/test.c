// Intraprocedural/Signedness 1

#include <stdlib.h>

void foo(unsigned int i)
{
	if(i < 0) static_assert(0);
	
}
