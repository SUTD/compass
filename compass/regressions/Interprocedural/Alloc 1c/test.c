// Interprocedural/Alloc 1c

#include<stdlib.h>

int* malloc_wrapper()
{
	return malloc(4);
}

void foo()
{
	int* x = malloc_wrapper();int* y =malloc_wrapper();

	*x = 4;
	*y = 5;
	static_assert(*x==4);
	static_assert(*y==5);



}
