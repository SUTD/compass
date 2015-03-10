// Interprocedural/Call 11

#include <stdlib.h>

void foo(int** x, int flag)
{
	*x = malloc(sizeof(int));
	if(flag)
		**x=1;
	else **x=2;
}

void bar(int flag)
{
	int* y;
	foo(&y, flag);
	if(flag) static_assert(*y == 1);
	else static_assert(*y == 2);
	
}
