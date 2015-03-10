// Intraprocedural/Signedness 4

#include <stdlib.h>

void foo(int* i)
{
	if(i<0) static_assert(0);
	
}
