// Intraprocedural/Address non-null

#include <stdlib.h>

void foo(int* i)
{
	int a =2;
	int* x = &a;
	static_assert(x!=NULL);
	
}
