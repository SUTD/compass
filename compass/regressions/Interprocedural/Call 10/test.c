// Interprocedural/Call 10

#include <stdlib.h>

void foo(int** x, int flag)
{
	*x = malloc(sizeof(int));
	if(flag)
		**x=1;
	else **x=2;
}

void bar()
{
	int* y;
	foo(&y, 1);
	static_assert(*y == 1);
	
}
