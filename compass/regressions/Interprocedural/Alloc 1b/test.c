// Interprocedural/Alloc 1b

#include<stdlib.h>

int* malloc_wrapper()
{
	return malloc(4);
}

void foo()
{
	int* x = malloc(4);int* y =malloc(4);

	*x = 4;
	*y = 5;
	static_assert(*x==4);
	static_assert(*y==5);



}
