// Intraprocedural/Call to unknown function 1

#include <stdlib.h>

void foo()
{
	int i=0;
	int x = bar();
	// should fail
	static_assert(x==1);
}