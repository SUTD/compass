// Intraprocedural/test assume array upgrade

#include <stdlib.h>

void foo(int flag, int** x)
{
	assume(*x != NULL);
	x[3] = 7;
	static_assert(*x!=NULL);
	
}
